#pragma once
#include "BleHelper.g.h"
#include "BluetoothLEDeviceDisplay.h"

namespace winrt::SDKTemplate::implementation
{
	struct BleHelper : BleHelperT<BleHelper>
	{
	public:
		BleHelper();
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> KnownDevices()
        {
            return m_knownDevices;
        }

        void StartBleDeviceWatcher();
        void StopBleDeviceWatcher();
        fire_and_forget Connect(hstring target);
        void Disconnect(hstring target);
        bool Not(bool value) { return !value; }
        event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler);  }
        void PropertyChanged(event_token const& token) noexcept { }

    private:
        SDKTemplate::MainPage rootPage() { return MainPage::Current(); }
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> m_knownDevices = single_threaded_observable_vector<Windows::Foundation::IInspectable>();
        std::vector<Windows::Devices::Enumeration::DeviceInformation> UnknownDevices;
        Windows::Devices::Enumeration::DeviceWatcher deviceWatcher{ nullptr };
        event_token deviceWatcherAddedToken;
        event_token deviceWatcherUpdatedToken;
        event_token deviceWatcherRemovedToken;
        event_token deviceWatcherEnumerationCompletedToken;
        event_token deviceWatcherStoppedToken;
        event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        Windows::Devices::Bluetooth::BluetoothLEDevice bluetoothLeDevice{ nullptr };
        void setDevice(Windows::Devices::Bluetooth::BluetoothLEDevice dev);
        Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic selectedCharacteristic{ nullptr };

        // Only one registered characteristic at a time.
        Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic registeredCharacteristic{ nullptr };
        Windows::Devices::Bluetooth::GenericAttributeProfile::GattPresentationFormat presentationFormat{ nullptr };


        std::tuple<SDKTemplate::BluetoothLEDeviceDisplay, uint32_t> FindBluetoothLEDeviceDisplay(hstring const& id);
        std::vector<Windows::Devices::Enumeration::DeviceInformation>::iterator FindUnknownDevices(hstring const& id);

        fire_and_forget DeviceWatcher_Added(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformation deviceInfo);
        fire_and_forget DeviceWatcher_Updated(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate);
        fire_and_forget DeviceWatcher_Removed(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate);
        fire_and_forget DeviceWatcher_EnumerationCompleted(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&);
        fire_and_forget DeviceWatcher_Stopped(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&);

        fire_and_forget ConnectDevice();
        fire_and_forget DisconnectDevice();
        static hstring m_curDevId;

    };
}

namespace winrt::SDKTemplate::factory_implementation
{
    struct BleHelper : BleHelperT<BleHelper, implementation::BleHelper>
    {
    };
}
