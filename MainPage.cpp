#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
#undef _HAS_CXX20

namespace winrt::SDKTemplate::implementation
{
    MainPage* MainPage::m_pCurrent;

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void AsynFunc() {
    }

    /// <summary>
    /// 非同期メソッドは fire_and_forget を戻り値として定義し、普通に呼び出せば実現できる
    /// co_await resume_background(); を冒頭で呼び出すのを定型とする。
    /// </summary>
    /// <param name="ms"></param>
    /// <returns></returns>
    fire_and_forget MainPage::StopWatcher(long ms) {
        co_await resume_background();
        Sleep(ms);
        m_bHelper.StopBleDeviceWatcher();
    }

    void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        btnSearch().Content(box_value(L"Searching..."));
        m_bHelper.StartBleDeviceWatcher();
        devList().ItemsSource(m_knownDevices);
        // 非同期メソッドを実行し、一定時間後にデバイスの検索をやめる
        StopWatcher(3000);
    }

    void MainPage::NotifyUser(hstring const& strMessage, NotifyType type)
    {
        switch (type) {
        case NotifyType::FoundMessage: {
            // ListBoxやListViewの配列クラスが受け付けるのはIInspectableインターフェースを持つオブジェクトだけなので、
            //　ただの文字列を渡す場合、winrt::box_valueメソッドでラッピングする必要がある
            m_knownDevices.Append(box_value(strMessage));
            }
            break;
        case NotifyType::ConnectMessage: {
            btnConnect().Content(box_value(L"Disconnect"));
        }
            break;
        case NotifyType::DiscMessage: {
            btnConnect().Content(box_value(L"Connect"));
            m_conDevice.clear();
        }
            break;
        case NotifyType::StatusMessage:
        case NotifyType::ErrorMessage: {
            StatusText().Text(strMessage);
        }
            break;

        }
    }

    static hstring selstr;

    void MainPage::ClickHandlerConnect()
    {
        auto lifetime = get_strong();
        // リストに何もない、モシクハ選択されていない場合何もしない
        if (m_knownDevices.Size() < 1 || devList().SelectedIndex() < 0)
            return;
        if (!m_conDevice.empty()) {
            btnConnect().Content(box_value(L"Disconnecting..."));
            m_bHelper.Connect(selstr);
            return;
        }
        // unbox化して中身を取り出す
        selstr = unbox_value<hstring>(devList().SelectedItem());

        // bluetoothID + "\t" + 名称なので分離してIDのみにする
        // hstringのままだと文字列操作できないので basicstringを使用する
        std::wstring starget(selstr.c_str());
        size_t idxTab = starget.find('\t');
        selstr = starget.substr(0, idxTab).c_str();

        btnConnect().Content(box_value(L"Connecting..."));
        m_bHelper.Connect(selstr);
        m_conDevice = selstr;
    }

}
