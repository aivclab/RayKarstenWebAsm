const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  mode: 'development',
  entry: './src/index.js',
  output: {
    filename: 'bundle.js',
    path: path.resolve(__dirname, 'dist')
  },

  module: 
  {
    rules: [ 
     { test: /\.css$/, use: [ 'style-loader', 'css-loader']},
     {test: /\.(png|jpg|gif)$/,
     use: [
       {
         loader: 'file-loader',
         options: {},
       },
     ]},
     ]
  },
  node: {
    fs: "empty"
},

  devServer : {
    host: '0.0.0.0',}
};