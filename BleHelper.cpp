#include "pch.h"
#include "BleHelper.h"
// 通常のヘッダーの後に、ランタイムクラス用にジェネレートされたCPPファイルをインクルードする必要がある
#include "BleHelper.g.cpp"
#include "SampleConfiguration.h"

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Devices::Enumeration;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization;
using namespace Windows::Security::Cryptography;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace SDKTemplate;
using namespace SDKTemplate::implementation;

namespace
{
    void SetVisibility(UIElement const& element, bool visible)
    {
        element.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
    }

    // Utility function to convert a string to an int32_t and detect bad input
    bool TryParseInt(const wchar_t* str, int32_t& result)
    {
        wchar_t* end;
        errno = 0;
        result = std::wcstol(str, &end, 0);

        if (str == end)
        {
            // Not parseable.
            return false;
        }

        if (errno == ERANGE || result < INT_MIN || INT_MAX < result)
        {
            // Out of range.
            return false;
        }

        if (*end != L'\0')
        {
            // Extra unparseable characters at the end.
            return false;
        }

        return true;
    }

    template<typename T>
    T Read(DataReader const& reader);

    template<>
    uint32_t Read<uint32_t>(DataReader const& reader)
    {
        return reader.ReadUInt32();
    }

    template<>
    int32_t Read<int32_t>(DataReader const& reader)
    {
        return reader.ReadInt32();
    }

    template<>
    uint8_t Read<uint8_t>(DataReader const& reader)
    {
        return reader.ReadByte();
    }

    template<typename T>
    bool TryExtract(IBuffer const& buffer, T& result)
    {
        if (buffer.Length() >= sizeof(T))
        {
            DataReader reader = DataReader::FromBuffer(buffer);
            result = Read<T>(reader);
            return true;
        }
        return false;
    }
}



namespace winrt
{
    hstring to_hstring(DevicePairingResultStatus status)
    {
        switch (status)
        {
        case DevicePairingResultStatus::Paired: return L"Paired";
        case DevicePairingResultStatus::NotReadyToPair: return L"NotReadyToPair";
        case DevicePairingResultStatus::NotPaired: return L"NotPaired";
        case DevicePairingResultStatus::AlreadyPaired: return L"AlreadyPaired";
        case DevicePairingResultStatus::ConnectionRejected: return L"ConnectionRejected";
        case DevicePairingResultStatus::TooManyConnections: return L"TooManyConnections";
        case DevicePairingResultStatus::HardwareFailure: return L"HardwareFailure";
        case DevicePairingResultStatus::AuthenticationTimeout: return L"AuthenticationTimeout";
        case DevicePairingResultStatus::AuthenticationNotAllowed: return L"AuthenticationNotAllowed";
        case DevicePairingResultStatus::AuthenticationFailure: return L"AuthenticationFailure";
        case DevicePairingResultStatus::NoSupportedProfiles: return L"NoSupportedProfiles";
        case DevicePairingResultStatus::ProtectionLevelCouldNotBeMet: return L"ProtectionLevelCouldNotBeMet";
        case DevicePairingResultStatus::AccessDenied: return L"AccessDenied";
        case DevicePairingResultStatus::InvalidCeremonyData: return L"InvalidCeremonyData";
        case DevicePairingResultStatus::PairingCanceled: return L"PairingCanceled";
        case DevicePairingResultStatus::OperationAlreadyInProgress: return L"OperationAlreadyInProgress";
        case DevicePairingResultStatus::RequiredHandlerNotRegistered: return L"RequiredHandlerNotRegistered";
        case DevicePairingResultStatus::RejectedByHandler: return L"RejectedByHandler";
        case DevicePairingResultStatus::RemoteDeviceHasAssociation: return L"RemoteDeviceHasAssociation";
        case DevicePairingResultStatus::Failed: return L"Failed";
        }
        return L"Code " + to_hstring(static_cast<int>(status));
    }

    hstring to_hstring(GattCommunicationStatus status)
    {
        switch (status)
        {
        case GattCommunicationStatus::Success: return L"Success";
        case GattCommunicationStatus::Unreachable: return L"Unreachable";
        case GattCommunicationStatus::ProtocolError: return L"ProtocolError";
        case GattCommunicationStatus::AccessDenied: return L"AccessDenied";
        }
        return to_hstring(static_cast<int>(status));
    }
}

namespace winrt::SDKTemplate::implementation
{
    constexpr guid SerialSvcGuid = { 0x0000ffe0, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
    constexpr guid SerialWriteChrGuid = { 0x0000ffe1, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
    constexpr guid SerialReadChrGuid = { 0x0000ffe2, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
    // This scenario uses a DeviceWatcher to enumerate nearby Bluetooth Low Energy devices,
    // displays them in a ListView, and lets the user select a device and pair it.
    // This device will be used by future scenarios.
    // For more information about device discovery and pairing, including examples of
    // customizing the pairing process, see the DeviceEnumerationAndPairing sample.

    hstring BleHelper::m_curDevId;

    BleHelper::BleHelper() {

    }


#pragma region UI Code
    /*

    void BleHelper::OnNavigatedFrom(NavigationEventArgs const&)
    {
        StopBleDeviceWatcher();

        // Save the selected device's ID for use in other scenarios.
        auto bleDeviceDisplay = ResultsListView().SelectedItem().as<SDKTemplate::BluetoothLEDeviceDisplay>();
        if (bleDeviceDisplay != nullptr)
        {
            SampleState::SelectedBleDeviceId = bleDeviceDisplay.Id();
            SampleState::SelectedBleDeviceName = bleDeviceDisplay.Name();
        }
    }

    void BleHelper::EnumerateButton_Click()
    {
        if (deviceWatcher == nullptr)
        {
            StartBleDeviceWatcher();
            EnumerateButton().Content(box_value(L"Stop enumerating"));
            rootPage().NotifyUser(L"Device watcher started.", NotifyType::StatusMessage);
        }
        else
        {
            StopBleDeviceWatcher();
            EnumerateButton().Content(box_value(L"Start enumerating"));
            rootPage().NotifyUser(L"Device watcher stopped.", NotifyType::StatusMessage);
        }
    }
    */
#pragma endregion

#pragma region Device discovery
    /// <summary>
    /// Starts a device watcher that looks for all nearby Bluetooth devices (paired or unpaired). 
    /// Attaches event handlers to populate the device collection.
    /// </summary>
    void BleHelper::StartBleDeviceWatcher()
    {
        // Additional properties we would like about the device.
        // Property strings are documented here https://msdn.microsoft.com/en-us/library/windows/desktop/ff521659(v=vs.85).aspx
        auto requestedProperties = single_threaded_vector<hstring>({ L"System.Devices.Aep.DeviceAddress", L"System.Devices.Aep.IsConnected", L"System.Devices.Aep.Bluetooth.Le.IsConnectable" });

        // BT_Code: Example showing paired and non-paired in a single query.
        hstring aqsAllBluetoothLEDevices = L"(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";

        deviceWatcher =
            Windows::Devices::Enumeration::DeviceInformation::CreateWatcher(
                aqsAllBluetoothLEDevices,
                requestedProperties,
                DeviceInformationKind::AssociationEndpoint);

        // Register event handlers before starting the watcher.
        deviceWatcherAddedToken = deviceWatcher.Added({ get_weak(), &BleHelper::DeviceWatcher_Added });
        deviceWatcherUpdatedToken = deviceWatcher.Updated({ get_weak(), &BleHelper::DeviceWatcher_Updated });
        deviceWatcherRemovedToken = deviceWatcher.Removed({ get_weak(), &BleHelper::DeviceWatcher_Removed });
        deviceWatcherEnumerationCompletedToken = deviceWatcher.EnumerationCompleted({ get_weak(), &BleHelper::DeviceWatcher_EnumerationCompleted });
        deviceWatcherStoppedToken = deviceWatcher.Stopped({ get_weak(), &BleHelper::DeviceWatcher_Stopped });

        // Start over with an empty collection.
        m_knownDevices.Clear();

        // Start the watcher. Active enumeration is limited to approximately 30 seconds.
        // This limits power usage and reduces interference with other Bluetooth activities.
        // To monitor for the presence of Bluetooth LE devices for an extended period,
        // use the BluetoothLEAdvertisementWatcher runtime class. See the BluetoothAdvertisement
        // sample for an example.
        deviceWatcher.Start();
    }

    /// <summary>
    /// Stops watching for all nearby Bluetooth devices.
    /// </summary>
    void BleHelper::StopBleDeviceWatcher()
    {
        if (deviceWatcher != nullptr)
        {
            // Unregister the event handlers.
            deviceWatcher.Added(deviceWatcherAddedToken);
            deviceWatcher.Updated(deviceWatcherUpdatedToken);
            deviceWatcher.Removed(deviceWatcherRemovedToken);
            deviceWatcher.EnumerationCompleted(deviceWatcherEnumerationCompletedToken);
            deviceWatcher.Stopped(deviceWatcherStoppedToken);

            // Stop the watcher.
            deviceWatcher.Stop();
            deviceWatcher = nullptr;
        }
    }

    std::tuple<SDKTemplate::BluetoothLEDeviceDisplay, uint32_t> BleHelper::FindBluetoothLEDeviceDisplay(hstring const& id)
    {
        uint32_t size = m_knownDevices.Size();
        for (uint32_t index = 0; index < size; index++)
        {
            auto bleDeviceDisplay = m_knownDevices.GetAt(index).as<SDKTemplate::BluetoothLEDeviceDisplay>();
            if (bleDeviceDisplay.Id() == id)
            {
                return { bleDeviceDisplay, index };
            }
        }
        return { nullptr, 0 - 1U };
    }

    std::vector<Windows::Devices::Enumeration::DeviceInformation>::iterator BleHelper::FindUnknownDevices(hstring const& id)
    {
        return std::find_if(UnknownDevices.begin(), UnknownDevices.end(), [&](auto&& bleDeviceInfo)
            {
                return bleDeviceInfo.Id() == id;
            });
    }

    fire_and_forget BleHelper::DeviceWatcher_Added(DeviceWatcher sender, DeviceInformation deviceInfo)
    {
        // We must update the collection on the UI thread because the collection is databound to a UI element.
        auto lifetime = get_strong();
        co_await resume_foreground(rootPage().Dispatcher());

        OutputDebugStringW((L"Added " + deviceInfo.Id() + deviceInfo.Name()).c_str());

        // Protect against race condition if the task runs after the app stopped the deviceWatcher.
        if (sender == deviceWatcher)
        {
            // Make sure device isn't already present in the list.
            if (std::get<0>(FindBluetoothLEDeviceDisplay(deviceInfo.Id())) == nullptr)
            {
                if (!deviceInfo.Name().empty())
                {
                    // If device has a friendly name display it immediately.
                    m_knownDevices.Append(make<BluetoothLEDeviceDisplay>(deviceInfo));
                    rootPage().NotifyUser(deviceInfo.Id() + L"\t" + deviceInfo.Name(), NotifyType::FoundMessage);
                }
                else
                {
                    // Add it to a list in case the name gets updated later. 
                    UnknownDevices.push_back(deviceInfo);
                }
            }
        }
    }

    fire_and_forget BleHelper::DeviceWatcher_Updated(DeviceWatcher sender, DeviceInformationUpdate deviceInfoUpdate)
    {
        // We must update the collection on the UI thread because the collection is databound to a UI element.
        auto lifetime = get_strong();
        co_await resume_foreground(rootPage().Dispatcher());

        OutputDebugStringW((L"Updated " + deviceInfoUpdate.Id()).c_str());

        // Protect against race condition if the task runs after the app stopped the deviceWatcher.
        if (sender == deviceWatcher)
        {
            SDKTemplate::BluetoothLEDeviceDisplay bleDeviceDisplay = std::get<0>(FindBluetoothLEDeviceDisplay(deviceInfoUpdate.Id()));
            if (bleDeviceDisplay != nullptr)
            {
                // Device is already being displayed - update UX.
                bleDeviceDisplay.Update(deviceInfoUpdate);
                co_return;
            }

            auto deviceInfo = FindUnknownDevices(deviceInfoUpdate.Id());
            if (deviceInfo != UnknownDevices.end())
            {
                deviceInfo->Update(deviceInfoUpdate);
                // If device has been updated with a friendly name it's no longer unknown.
                if (!deviceInfo->Name().empty())
                {
                    m_knownDevices.Append(make<BluetoothLEDeviceDisplay>(*deviceInfo));
                    UnknownDevices.erase(deviceInfo);
                }
            }
        }
    }

    fire_and_forget BleHelper::DeviceWatcher_Removed(DeviceWatcher sender, DeviceInformationUpdate deviceInfoUpdate)
    {
        // We must update the collection on the UI thread because the collection is databound to a UI element.
        auto lifetime = get_strong();
        co_await resume_foreground(rootPage().Dispatcher());

        OutputDebugStringW((L"Removed " + deviceInfoUpdate.Id()).c_str());

        // Protect against race condition if the task runs after the app stopped the deviceWatcher.
        if (sender == deviceWatcher)
        {
            // Find the corresponding DeviceInformation in the collection and remove it.
            auto [bleDeviceDisplay, index] = FindBluetoothLEDeviceDisplay(deviceInfoUpdate.Id());
            if (bleDeviceDisplay != nullptr)
            {
                m_knownDevices.RemoveAt(index);
            }

            auto deviceInfo = FindUnknownDevices(deviceInfoUpdate.Id());
            if (deviceInfo != UnknownDevices.end())
            {
                UnknownDevices.erase(deviceInfo);
            }

            if (m_curDevId == deviceInfoUpdate.Id()) {
                DisconnectDevice();
            }
        }
    }

    fire_and_forget BleHelper::DeviceWatcher_EnumerationCompleted(DeviceWatcher sender, IInspectable const&)
    {
        // Access this->deviceWatcher on the UI thread to avoid race conditions.
        auto lifetime = get_strong();
        co_await resume_foreground(rootPage().Dispatcher());

        // Protect against race condition if the task runs after the app stopped the deviceWatcher.
        if (sender == deviceWatcher)
        {
            rootPage().NotifyUser( to_hstring(m_knownDevices.Size()) + L" devices found. Enumeration completed.", NotifyType::StatusMessage);
        }
    }

    fire_and_forget BleHelper::DeviceWatcher_Stopped(DeviceWatcher sender, IInspectable const&)
    {
        // Access this->deviceWatcher on the UI thread to avoid race conditions.
        auto lifetime = get_strong();
        co_await resume_foreground(rootPage().Dispatcher());

        // Protect against race condition if the task runs after the app stopped the deviceWatcher.
        if (sender == deviceWatcher)
        {
            rootPage().NotifyUser(L"No longer watching for devices.",
                sender.Status() == DeviceWatcherStatus::Aborted ? NotifyType::ErrorMessage : NotifyType::StatusMessage);
        }
    }
#pragma endregion

#pragma region Pairing
    void BleHelper::setDevice(Windows::Devices::Bluetooth::BluetoothLEDevice dev ) {
        bluetoothLeDevice = dev;
    }

    fire_and_forget BleHelper::ConnectDevice() {
        auto lifetime = get_strong();
        try
        {
            // BT_Code: BluetoothLEDevice.FromIdAsync must be called from a UI thread because it may prompt for consent.
            bluetoothLeDevice = co_await BluetoothLEDevice::FromIdAsync(m_curDevId);

            if (bluetoothLeDevice == nullptr)
            {
                rootPage().NotifyUser(L"Failed to connect to device.", NotifyType::ErrorMessage);
            }
        }
        catch (hresult_error& ex)
        {
            if (ex.to_abi() == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_AVAILABLE))
            {
                rootPage().NotifyUser(L"Bluetooth radio is not on.", NotifyType::ErrorMessage);
            }
            else
            {
                throw;
            }
        }
        co_return;
    }
    
    /// <summary>
    /// 接続要求実行
    /// </summary>
    /// <param name="target"></param>
    fire_and_forget BleHelper::Connect(hstring target) {
        auto lifetime = get_strong();
        hstring  sDevId;
        if (!m_curDevId.empty()) {
            bool isCloseOnly = target == m_curDevId;
            DisconnectDevice();
            if (isCloseOnly)
                co_return;
        }
        for (auto it = m_knownDevices.begin(); it != m_knownDevices.end(); it++) {
            auto dev = (*it).as< BluetoothLEDeviceDisplay>();
            if (dev->Id() == target) {
                m_curDevId = dev->Id();
                break;
            }
        }
        if (m_curDevId.empty())
            co_return;                  // fire_and_forget関数なので、co_waitを呼ばない時は co_returnする
        bluetoothLeDevice = co_await  BluetoothLEDevice::FromIdAsync(m_curDevId);
        if (bluetoothLeDevice == nullptr)
        {
            rootPage().NotifyUser(L"Failed to connect to device.", NotifyType::ErrorMessage);
        }
        else {
            rootPage().NotifyUser(L"Connected", NotifyType::ConnectMessage);
            DeviceAccessStatus  dasts = co_await bluetoothLeDevice.RequestAccessAsync();
            GattDeviceServicesResult servicesResult = co_await  bluetoothLeDevice.GetGattServicesForUuidAsync(SerialSvcGuid, BluetoothCacheMode::Uncached);
            //auto service = bluetoothLeDevice.GetGattService(SerialSvcGuid);
            if (servicesResult.Status() == GattCommunicationStatus::Success)
            {
                auto service = servicesResult.Services().GetAt(0);
                
                GattCharacteristicsResult characteristicsResult =
                    co_await service.GetCharacteristicsForUuidAsync(SerialWriteChrGuid);

                if (characteristicsResult.Status() == GattCommunicationStatus::Success) {
                    GattCharacteristic  writeChr = characteristicsResult.Characteristics().GetAt(0);
                    DataWriter writer;
                    const char* pszData ="apple";
                    std::vector<unsigned char> vdata;
                    for(int i = 0; *pszData; pszData++) {
                        vdata.push_back(*pszData);
                    }
                    writer.WriteBytes(vdata);

                    GattCommunicationStatus cresult = co_await writeChr.WriteValueAsync(writer.DetachBuffer());

                    //成功
                    if (cresult == GattCommunicationStatus::Success)
                    {
                    }
                 }
            }
        }
    }

    fire_and_forget BleHelper::DisconnectDevice() {
        if (m_curDevId.empty())
            co_return;                  // fire_and_forget関数なので、co_waitを呼ばない時は co_returnする
        bluetoothLeDevice.Close();
        bluetoothLeDevice = nullptr;
        m_curDevId.clear();
        rootPage().NotifyUser(L"Disconnected device.", NotifyType::DiscMessage);
       
        co_return;
    }

#pragma endregion
}
