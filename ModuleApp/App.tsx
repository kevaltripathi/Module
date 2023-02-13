/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 */

import React from 'react';
import {Box, extendTheme, NativeBaseProvider, Flex, Center, Spacer} from "native-base";
import {appTheme} from './styles/appStyles'
import {ConnectDevice } from "./components/ConnectDevice";
import {Message} from "./components/Message";
import {
    SafeAreaView,
    ScrollView,
    StatusBar,
    StyleSheet,
    Text,
    useColorScheme,
    View,
} from 'react-native';

const App = (): JSX.Element => {
    return (
        <NativeBaseProvider theme={appTheme}>
            <SafeAreaView>
                <StatusBar/>
                <ScrollView>
                    <Flex direction="column" mb="2.5" mt="1.5" h='100%'>
                        <Center>
                            <Message/>
                        </Center>
                        <Spacer/>
                        <Center>
                            <ConnectDevice/>
                        </Center>
                    </Flex>
                </ScrollView>
            </SafeAreaView>
        </NativeBaseProvider>

    );
}

export default App;