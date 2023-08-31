const webpack = require('webpack');
const TSConfigPathsPlugin = require('tsconfig-paths-webpack-plugin');

require('ts-loader');
require('style-loader');
require('css-loader');
require('less-loader');
require('svg-sprite-loader');
require('svgo-loader');
require('less');

const TS_CONFIG_PATH = './tsconfig.json';

module.exports = {
    mode: 'development',
    //A quick pack to speed up debugging build times while maintaining references
    //to source files to follow in stack traces
    devtool: 'cheap-eval-source-map',
    //2 entry points for this project, 'designer' is the library entry and
    //'client' is the debug client entry.
    entry: {
        'client': './src/index.tsx'
    },
    //Output a umd bundle for consumption as amd or commonjs
    output: {
        filename: '[name].js',
        library: '[name]',
        libraryTarget: 'umd'
    },
    resolve: {
        extensions: ['.ts', '.tsx', '.js', '.css', '.svg'],
        plugins: [
            new TSConfigPathsPlugin({configFile: TS_CONFIG_PATH})
        ]
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                loader: 'ts-loader'
            },
            {
                test: /\.(le|c)ss$/,
                use: [{
                    loader: 'style-loader'
                }, {
                    loader: 'css-loader'
                }, {
                    loader: 'less-loader',
                    options: {}
                }]
            },
            {
                test: /\.svg$/,
                use: [{
                    loader: 'svg-sprite-loader'
                }, {
                    loader: 'svgo-loader'
                }]
            }
        ]
    },
    plugins: [
        new webpack.DefinePlugin({
            'DEBUG': true
        })
    ]
};
