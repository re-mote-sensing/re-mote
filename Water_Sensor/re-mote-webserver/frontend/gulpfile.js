const gulp = require('gulp');
const del = require('del');
const ts = require('gulp-typescript');
const webpackStream = require('webpack-stream');
const webpack = require('webpack');

const BUILD_DIR = 'build/';
const DIST_DIR = 'dist/';

gulp.task('default', ['clean'], function () {
    return gulp.start(['bundle:production', 'static']);
});

gulp.task('compile', function () {
    const tsProject = ts.createProject('tsconfig.json');
    return tsProject.src()
        .pipe(tsProject()).js
        .pipe(gulp.dest(BUILD_DIR));
});

gulp.task('bundle', function () {
    const tsProject = ts.createProject('tsconfig.json');
    const webpackConfig = require('./webpack.config');
    return tsProject.src()
        .pipe(tsProject()).js
        .pipe(webpackStream(webpackConfig, webpack))
        .pipe(gulp.dest(DIST_DIR));
});

gulp.task('bundle:production', function () {
    const tsProject = ts.createProject('tsconfig.json');
    const webpackConfig = require('./webpack-production.config');
    return tsProject.src()
        .pipe(tsProject()).js
        .pipe(webpackStream(webpackConfig, webpack))
        .pipe(gulp.dest(DIST_DIR));
});

gulp.task('static', function () {
    return gulp.src('public/**/*')
        .pipe(gulp.dest(DIST_DIR));
});

gulp.task('clean', function () {
    return gulp.start(['clean:build', 'clean:dist']);
});

gulp.task('clean:build', function () {
    return del([BUILD_DIR + '**/*']);
});

gulp.task('clean:dist', function () {
    return del([DIST_DIR + '**/*']);
});
