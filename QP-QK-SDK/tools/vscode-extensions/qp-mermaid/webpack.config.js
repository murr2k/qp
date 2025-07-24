'use strict';

const path = require('path');
const webpack = require('webpack');

/** @typedef {import('webpack').Configuration} WebpackConfig **/

/** @type WebpackConfig */
const webExtensionConfig = {
    mode: 'none',
    target: 'webworker',
    entry: {
        extension: './src/extension.ts',
    },
    output: {
        filename: '[name].js',
        path: path.join(__dirname, './out/web'),
        libraryTarget: 'commonjs',
    },
    resolve: {
        mainFields: ['browser', 'module', 'main'],
        extensions: ['.ts', '.js'],
        alias: {},
        fallback: {
            assert: require.resolve('assert'),
        },
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                exclude: /node_modules/,
                use: [
                    {
                        loader: 'ts-loader',
                    },
                ],
            },
        ],
    },
    plugins: [
        new webpack.ProvidePlugin({
            process: 'process/browser',
        }),
    ],
    externals: {
        vscode: 'commonjs vscode',
    },
    performance: {
        hints: false,
    },
    devtool: 'nosources-source-map',
};

/** @type WebpackConfig */
const nodeExtensionConfig = {
    mode: 'none',
    target: 'node',
    entry: {
        extension: './src/extension.ts',
    },
    output: {
        filename: '[name].js',
        path: path.join(__dirname, './out'),
        libraryTarget: 'commonjs2',
    },
    resolve: {
        extensions: ['.ts', '.js'],
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                exclude: /node_modules/,
                use: [
                    {
                        loader: 'ts-loader',
                    },
                ],
            },
        ],
    },
    externals: {
        vscode: 'commonjs vscode',
    },
    devtool: 'nosources-source-map',
};

module.exports = [webExtensionConfig, nodeExtensionConfig];