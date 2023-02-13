import React from 'react';
import {extendTheme} from "native-base";

export const appTheme = extendTheme({
    root:{
        bg: '#F6BB42',
    },
    colors: {
        sunflower: {
            50: '#FFCE54',
            100: '#F6BB42',
        },
        darkgrey: {
            50: '#656D78',
            100: '#434a54',
        },
        grass: {
            50: '#A0D468',
            100: '#8CC152',
        },
        pinkrose: {
            50: '#EC87C0',
            100: '#D770AD',
        },
    },
    components: {
        Button: {
            baseStyle: {
                size: 'md',
            },

            defaultProps: {
                colorScheme: 'lime'
            }
        },
        TextArea: {
            baseStyle: {
                h: 150,
                maxW: '300',
            },

            defaultProps: {

            }
        }
    },
    config: {
        initialColorMode: 'dark',
    },
});