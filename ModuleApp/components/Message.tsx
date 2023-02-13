import React, {useState} from 'react';
import {Box, TextArea, NativeBaseProvider, Button, VStack, Center} from "native-base";
import {appTheme} from '../styles/appStyles'

export const Message = () => {

    const [textAreaValue, setTextAreaValue] = useState(null);
    return (
        <NativeBaseProvider theme={appTheme}>
            <VStack space={2} w='100%'>
                <Center w="100%">
                    <TextArea placeholder={"Write your message!"} autoCompleteType={undefined} onChangeText={text => setTextAreaValue(text)}/>
                    </Center>
                <Center w="100%">
                    <Button>Send message</Button>
                </Center>
            </VStack>
        </NativeBaseProvider>
    )
}