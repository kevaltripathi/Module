import React, {useState} from 'react';
import {Box, NativeBaseProvider, Button} from "native-base";
import {appTheme} from '../styles/appStyles'
import {BleManager} from "react-native-ble-plx";

export const ConnectDevice = () => {
    const manager = new BleManager();

    const [isScanning, setIsScanning] = useState(false);

    const scanAndConnect = () => {
        setIsScanning(true)
        manager.startDeviceScan(null, null, (error, device) => {
            if (error) {
                console.warn(error);
                return
            }

            if (device?.name === 'Module BLE') {
                // Stop scanning
                manager.stopDeviceScan();
                // Proceed with connection
                const connectedDevice = device.connect()
            }

            // Stop scanning after 10 seconds
            setTimeout(() => {
                manager.stopDeviceScan();
                setIsScanning(false);
            }, 10000);
        });
    }

    const componentWillMount = () => {
        const subscription = manager.onStateChange((state) => {
            if (state === 'PoweredOn') {
                scanAndConnect();
                subscription.remove();
            }
        }, true);
    }

    return (
        <NativeBaseProvider theme={appTheme}>
            <Box alignItems='center'>
                <Button onPress={componentWillMount}>Connect to device!</Button>
            </Box>
        </NativeBaseProvider>
    )
}