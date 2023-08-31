const webpack = require('webpack');

const devConfig = require('./webpack.config');
module.exports = Object.assign(devConfig, {
    mode: 'production',
    devtool: false,
    optimization: {
        minimize: true
    },
    plugins: [
        //Define the variable 'process.env.NODE_ENV' to 'production' so that
        //the uglify phase will omit if statements with that check as dead code.
        //This is used to drop debug statements/warnings.
        new webpack.DefinePlugin({
            'process.env.NODE_ENV': JSON.stringify('production'),
            'DEBUG': false
        })
    ],
});
