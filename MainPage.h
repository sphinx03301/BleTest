#pragma once

#include "MainPage.g.h"

//
/**
* Windows CPP/WinRTの学習兼用プロジェクト
* Step1 Windows2019の場合、拡張機能の追加でオンラインからWinRTで検索し、CPP/WinRT templates and visualizer
*       の拡張機能を追加する。2022の場合は最初から入っているので追加しなくてOK,2017は無理
* Step2 CPP/WinRTのテンプレートで空のプロジェクト作成
*
☆ 全般
   ・C++なので、プロパティ系はメソッドでアクセスする。同名のメソッドで引数無しはgetter
     引数有をsetterとする
   ★Bluetoothを使用しているので、package.appxmanifestを開き、機能タブよりbluetoothの機能をONにすること
☆ Stringへのバインド
    midl ファイルにString型のプロパティを追加し、runtimeclassの実装クラスにプロパティを追加する
    hstring propname();
    void propname(hstring const& value)
    xaml側は Text={x:Bind propname, Mode=TwoWay} でやり取りできる。
☆ コントロールへのアクセス
    box_value メソッドでカプセル化する





    bluetoothについて

    AndroidのnRF Connectツールで調査したところ、JDY-23 およびBT-05のBLEシリアル変換ボードの
    サービスGUIDは FFE0　であり、Characteristic はFFE1であった。
    しかしWIndowsでは FFE0のサービスにアクセスするとエラーとなってしまい、通信できない。


*/

namespace winrt::SDKTemplate::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            m_pCurrent = this;
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
        hstring Text1() { return _Text1; }
        void Text1(const hstring& value) { _Text1 = value; }

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
        void ClickHandlerConnect();
        void NotifyUser(hstring const& strMessage, NotifyType type);
        static winrt::SDKTemplate::MainPage Current() { return *m_pCurrent; }
    private:
        static MainPage* m_pCurrent;
        hstring  _Text1{ L"" };
        Windows::Devices::Bluetooth::BluetoothLEDevice bluetoothLeDevice{ nullptr };
        /// <summary>
        /// BlueToothAPI実行のクラスインスタンス
        /// </summary>
        BleHelper m_bHelper;
        hstring  m_conDevice;
        fire_and_forget StopWatcher(long ms);
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> m_knownDevices = single_threaded_observable_vector<Windows::Foundation::IInspectable>();
    };
}

namespace winrt::SDKTemplate::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
