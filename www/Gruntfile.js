'use strict';

module.exports = function(grunt) {

    // Project configuration.
    grunt.initConfig({
        dirs: {
            dest: 'dist',
            src: 'source'
        },

        // Copy files that don't need compilation to dist/
        copy: {
            dist: {
                files: [{
                    dest: '<%= dirs.dest %>/',
                    src: ['*', '!*.html', 'google*.html'],
                    filter: 'isFile',
                    expand: true,
                    cwd: '<%= dirs.src %>/'
                }, {
                    dest: '<%= dirs.dest %>/',
                    src: '.htaccess',
                    expand: true,
                    cwd: '<%= dirs.src %>/'
                }, {
                    dest: '<%= dirs.dest %>/',
                    src: ['img/**', '!**/_old/**'],
                    expand: true,
                    cwd: '<%= dirs.src %>/'
                }, {
                    dest: '<%= dirs.dest %>/',
                    src: ['js/*.min.js', 'js/prettify/**'],
                    expand: true,
                    cwd: '<%= dirs.src %>/'
                }]
            }
        },

        includereplace: {
            dist: {
                options: {
                    globals: {
                        DATE: '<%= grunt.template.today("dddd, mmmm dS, yyyy, HH:MM:ss Z") %>',
                        bottomHtml: '',
                        headHtml: '',
                        metaDescription: '',
                        metaKeywords: ''
                    }
                },
                files: [{
                    src: ['*.html', '!**/google*.html'],
                    dest: '<%= dirs.dest %>/',
                    expand: true,
                    cwd: '<%= dirs.src %>/'
                }]
            }
        },

        concat: {
            prettify: {
                src: '<%= dirs.src %>/css/prettify.css',
                dest: '<%= dirs.dest %>/css/prettify.min.css'
            },
            css: {
                src: ['<%= dirs.src %>/css/normalize.css',
                      '<%= dirs.src %>/css/baguetteBox.css',
                      '<%= dirs.src %>/css/style.css'
                ],
                dest: '<%= dirs.dest %>/css/pack.css'
            },
            js: {
                src: ['<%= dirs.src %>/js/plugins.js',
                      '<%= dirs.src %>/js/baguetteBox.js',
                      '<%= dirs.src %>/js/baguetteBox-init.js',
                      '<%= dirs.src %>/js/google-analytics.js'
                ],
                dest: '<%= dirs.dest %>/js/pack.js'
            }
        },

        autoprefixer: {
            options: {
                browsers: [
                    'last 2 version',
                    '> 1%',
                    'Edge >= 12',
                    'Explorer >= 9',
                    'Firefox ESR',
                    'iOS >= 8',
                    'Safari >= 8'
                ]
            },
            pack: {
                src: '<%= concat.css.dest %>',
                dest: '<%= concat.css.dest %>'
            }
        },

        uncss: {
            options: {
                htmlroot: '<%= dirs.dest %>',
                ignore: [
                    /(#|\.)baguetteBox(\-[a-zA-Z]+)?/,
                    '#scrollUp',
                    '.ad-inpage'
                ],
                ignoreSheets: [/fonts.googleapis/, /www.google.com/],
                stylesheets: ['css/pack.css']
            },
            dist: {
                src: '<%= dirs.dest %>/**/*.html',
                dest: '<%= concat.css.dest %>'
            }
        },

        cssmin: {
            options: {
                level: {
                    1: {
                        specialComments: 0
                    }
                }
            },
            prettify: {
                src: '<%= concat.prettify.dest %>',
                dest: '<%= concat.prettify.dest %>'
            },
            css: {
                src: '<%= concat.css.dest %>',
                dest: '<%= concat.css.dest %>'
            }
        },

        uglify: {
            options: {
                compress: true,
                mangle: true,
                output: {
                    comments: false
                }
            },
            dist: {
                files: {
                    '<%= concat.js.dest %>': '<%= concat.js.dest %>'
                }
            }
        },

        htmlmin: {
            dist: {
                options: {
                    collapseBooleanAttributes: true,
                    collapseInlineTagWhitespace: false,
                    collapseWhitespace: true,
                    conservativeCollapse: false,
                    decodeEntities: true,
                    minifyCSS: {
                        level: {
                            1: {
                                specialComments: 0
                            }
                        }
                    },
                    minifyJS: true,
                    minifyURLs: false,
                    processConditionalComments: true,
                    removeAttributeQuotes: true,
                    removeComments: true,
                    removeOptionalAttributes: true,
                    removeOptionalTags: true,
                    removeRedundantAttributes: true,
                    removeScriptTypeAttributes: true,
                    removeStyleLinkTypeAttributes: true,
                    removeTagWhitespace: false,
                    sortAttributes: true,
                    sortClassName: true
                },
                expand: true,
                cwd: '<%= dirs.dest %>',
                dest: '<%= dirs.dest %>',
                src: ['**/*.html', '!**/google*.html']
            }
        },

        filerev: {
            css: {
                src: '<%= dirs.dest %>/css/**/{,*/}*.css'
            },
            js: {
                src: [
                    '<%= dirs.dest %>/js/**/{,*/}*.js',
                    '!<%= dirs.dest %>/js/prettify/lang-*.js'
                ]
            },
            images: {
                src: [
                    '<%= dirs.dest %>/img/**/*.{jpg,jpeg,gif,png,svg}'
                ]
            }
        },

        useminPrepare: {
            html: '<%= dirs.dest %>/index.html',
            options: {
                dest: '<%= dirs.dest %>',
                root: '<%= dirs.dest %>'
            }
        },

        usemin: {
            css: '<%= dirs.dest %>/css/pack*.css',
            html: '<%= dirs.dest %>/**/*.html'
        },

        sitemap: {
            dist: {
                pattern: ['<%= dirs.dest %>/**/*.html', '!<%= dirs.dest %>/**/google*.html'],
                siteRoot: '<%= dirs.dest %>/'
            }
        },

        connect: {
            options: {
                hostname: 'localhost',
                livereload: 35729,
                port: 8001
            },
            livereload: {
                options: {
                    base: '<%= dirs.dest %>/',
                    open: true  // Automatically open the webpage in the default browser
                }
            }
        },

        watch: {
            options: {
                livereload: '<%= connect.options.livereload %>'
            },
            dev: {
                files: ['<%= dirs.src %>/**', '.csslintrc', '.jshintrc', 'Gruntfile.js'],
                tasks: 'dev'
            },
            build: {
                files: ['<%= dirs.src %>/**', '.csslintrc', '.jshintrc', 'Gruntfile.js'],
                tasks: 'build'
            }
        },

        clean: {
            dist: '<%= dirs.dest %>/'
        },

        csslint: {
            options: {
                csslintrc: '.csslintrc'
            },
            src: [
                '<%= dirs.src %>/css/style.css'
            ]
        },

        jshint: {
            options: {
                jshintrc: '.jshintrc'
            },
            files: {
                src: ['Gruntfile.js', '<%= dirs.src %>/js/plugins.js']
            }
        },

        htmllint: {
            options: {
                ignore: [
                    /This document appears to be written in/
                ]
            },
            src: ['<%= dirs.dest %>/**/*.html', '!<%= dirs.dest %>/**/google*.html']
        },

        'gh-pages': {
            options: {
                base: '<%= dirs.dest %>'
            },
            src: ['**']
        }
    });

    // Load any grunt plugins found in package.json.
    require('load-grunt-tasks')(grunt, { scope: 'devDependencies' });
    require('time-grunt')(grunt);

    grunt.registerTask('dev', [
        'clean',
        'copy',
        'includereplace',
        'useminPrepare',
        'concat',
        'autoprefixer',
        'filerev',
        'usemin',
        'sitemap'
    ]);

    grunt.registerTask('build', [
        'clean',
        'copy',
        'includereplace',
        'sitemap',
        'useminPrepare',
        'concat',
        'autoprefixer',
        'uncss',
        'cssmin',
        'uglify',
        'filerev',
        'usemin',
        'htmlmin'
    ]);

    grunt.registerTask('test', [
        'build',
        'csslint',
        'jshint',
        'htmllint'
    ]);

    grunt.registerTask('server', [
        'build',
        'connect',
        'watch:build'
    ]);

    grunt.registerTask('deploy', [
        'build',
        'gh-pages'
    ]);

    grunt.registerTask('default', [
        'dev',
        'connect',
        'watch:dev'
    ]);

};
