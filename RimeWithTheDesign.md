﻿#summary La Rime 架構設計解讀
#labels Phase-Design

# 概念解讀 #

## 框架 ##

輸入法工作於需要輸入文本的程序中，後者就叫他做輸入法的「客戶程序」吧。

如果輸入法同時在不同的程序中工作，每個客戶程序裡的輸入內容都是不同的。
所以需要為每個客戶維護輸入法狀態，術語稱輸入法「上下文／Context」。

邏輯複雜的輸入法，常常有個獨立的程序來做運算，便於集中管理詞庫等資源。
Rime稱其為「服務／Service」，形式為一個無介面表現的後臺程序／Backend。

於是輸入法的「前端／Frontend」，即輸入法在客戶程序中的那部份，可依託於後臺的服務來運作，自身只需關注與操作系統交互，以及將消息向服務轉發。

Rime的Frontend/Backend模型，依照ibus、IMK等輸入法框架來設計：Frontend不含輸入邏輯，甚至不負責繪製輸入法介面。因此會比較容易適配現有的輸入法框架，不需要自己寫很多代碼。

在服務中，會為每一個客戶建立一個輸入法「會話／Session」。從功能上講，會話是將一系列按鍵消息變（Convert）為文字的問（Request）答（Response）過程。
技術上講，會話會負責搞定輸入法前端與服務之間的跨進程通信，同時在服務端為前端所代表的客戶分配必要的資源。這資源主要是指一部有狀態的、懂得所有輸入轉換邏輯的輸入法引擎「Engine」。

現在所寫的Rime庫，不做那框架部份，專注於輸入引擎的實現。
名字也叫「中州韻輸入法引擎／RIME」嘛。

這部份定義在代碼目錄 librime/ 、C++ namespace rime 裡面。
拋開實際的輸入法框架，俺先寫個控制台程序 `RimeConsole` 來模擬輸入，觀察Engine的輸出，以驗證其功能是否符合設計。

## 引擎 ##

輸入法的轉換邏輯自然是比較複雜，需要許多元件協同工作。要協同工作，比得組裝到一起，外面再加個殼。通常的機器都是如此，有外殼做封裝，看不到內部的元件，通過有限的幾處機關來操作，用起來比較省心。

Engine 對象，便是 Session 需要直接操作的介面。
除此之外，Rime庫還對外提供若干用於輸入輸出的數據對象。
包括他所持有的代表輸入法狀態的「上下文／Context」對象、
描述一份「輸入方案／Schema」的配置對象、
代表輸入按鍵的 `KeyEvent` 對象等。

開發者對 Engine 的期望，一是將來可不斷通過添加內部組件的方式增益其所不能，二是有能力動態地調整組件的調度，以在會話中切換到不同的輸入方式。

為了避免知道得太多，這引擎的內部構造必須精巧，他存在的意義在於接合內部的各種組件、並對外提供可靠的接口：Engine所表達的邏輯僅限於此。

標準化Engine內部的「組件／Component」，明確與關鍵組件之間的對接方式，就可以滿足可擴展、可配置這兩點期望。

# 設計與實現 #

## 組件 ##

為有好的擴展能力，定義一個接口來表示具有某種能力的一類組件；
為了達到可在運行時動態配置的目的，採取抽象工廠的設計模式。

`boost::factory`要求較高版本的Boost庫，所以我還是用了自家釀造的一套設施來做組件的包裝。

原則是，Rime中完成特定功能的對象，若只有一種實現方法，就寫個C++類／class；若有多種實現方法，就寫個「Rime類」／`rime::Class`。示意：
```
class Processor : public Class<Processor, Engine*> {
 public:
  Processor(Engine *engine) : engine_(engine) {}
  virtual ~Processor() {}
  // Processor的功能是處理按鍵消息
  virtual bool ProcessKeyEvent(const KeyEvent &ke);
 private:
  Engine *engine_;
};
```

然後，大家就可以寫各式Processor的實現啦……
當然，還要把每一種實現註冊為具名的「組件」。
```
class ToUpperCase : public Processor {
 public:
  ToUpperCase(Engine *engine);
  virtual bool ProcessKeyEvent(const KeyEvent &ke) {
    char ch = ke.ToAscii();
    if (islower(ch)) {
      engine_->CommitChar(ch - 'a' + 'A');
      return true;
    }
    return false;
  }
};

void RegisterRimeComponents() {
  Registry.instance().Register("upper", new Component<ToUpperCase>);
  // 註冊各種組件到Registry...
}
```

用法：
```
void UsingAProcessor() {
  // Class<>模板提供了簡便的方法，按名稱取得組件
  // Class<T>::Require() 從Registry中取得T::Component的指針
  // 而Component<T>繼承自T::Component，並實現了其中的純虛函數Create()
  Processor::Component* component = Processor::Require("upper");
  // 利用組件生成所需的對象
  KeyEvent ke;    // 輸入
  Engine engine;  // 上屏文字由此輸出
  Processor* processor = component->Create(&engine);
  bool taken = processor->ProcessKeyEvent(ke);
  delete processor;
}
```
實際的代碼中，會將這個例子改造成Engine持有Processor對象，並轉發輸入按鍵給Processor。

## 框架級組件和基礎組件 ##

以Engine的視角，可以把各種組件分為框架級組件和基礎組件。前者會由Engine創建並直接調用，故需要為每一類組件定義明確的接口。後者作為實現具體功能的積木，由框架級組件的實現類使用。

### 框架級組件 ###

目前設計中規劃了三類框架級組件：

  * Processor，處理按鍵，編輯輸入串
  * Segmentor，解釋輸入串「是什麼」，將輸入串分段，標記輸入內容的可能類型
  * Translator，翻譯一段輸入，給出一組備選結果

對於每一類組件，Engine將根據Schema中的配置，創建若干實例，並按一定規則調度，從而將複雜的輸入法邏輯分解到組件的各種實現裡，按輸入方案的需求組合。

當Engine接到前端傳來的按鍵消息，會順次調用一組Processor的ProcessKeyEvent()方法。
在這方法裡，每個Processor可決定是接受並處理該按鍵，放棄該按鍵還給系統做默認處理，還是留給其他的Processor來決定。

Processor的處理結果表現為對Context的修改。當某個Processor修改了Context的編碼串時，Engine獲得信號通知，開始切分和翻譯的流程。

首先按次序由各Segmentor嘗試對編碼串分段。通過N個回合，將編碼串分為N個編碼段。每一回合中，各Segmentor給出從編碼串指定位置開始可識別的最長編碼序列，及對應的編碼類型標籤。回合中最長的分段將被採納，將該編碼段的始末位置及一組編碼類型標籤記入Context中的分段信息。優先級較高的Segmentor也可以中止當前回合從而跳過優先級較低的Segmentor。

接下來，對每一個編碼段，調用各Translator做翻譯。Translator可憑藉編碼類型標籤來判斷本Translator能否完成該編碼段的翻譯。

每個Translator針對一個編碼段的翻譯結果為Translation對象，是用來取得一組候選結果的迭代器。同一編碼段由不同Translator給出的Translations，存入Menu對象。為了在有大量候選結果的情況下保持效率，Menu僅在需要時，譬如向後翻頁時，才會從Translation中取得一定數目的候選結果。
Translation有和其他Translation比較的方法，用於根據下一個候選結果的內容、或某種預定的策略來決定候選結果的排序。

Context中的Composition，匯總了分段信息、使用者在各代碼段對應的Menu中所選結果等信息。經過使用者手動確認結果、且未發生編輯動作的編碼段，將不再做重新分段的處理。

### 基礎組件 ###

目前需求明確的基礎組件包括：

  * Dictionary, 詞典，由編碼序列檢索候選結果
    * StaticDictionary, 靜態詞典的實現
    * DynamicDictionary, 動態詞典的實現
  * Prism, 音節拼寫至詞典編碼的映射
  * Algebra, 拼寫運算規則
  * Syllablizer, 音節切分算法