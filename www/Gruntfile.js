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
                files: [
                    {dest: '<%= dirs.dest %>/', src: ['*', '!*.html'], filter: 'isFile', expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: '.htaccess', expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: ['img/**', '!**/_old/**'], expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: 'js/*.min.js', expand: true, cwd: '<%= dirs.src %>/'}
                ]
            }
        },

        includereplace: {
            dist: {
                options: {
                    globals: {
                        metaDescription: '',
                        metaKeywords: ''
                    }
                },
                files: [
                    {src: '*.html', dest: '<%= dirs.dest %>/', expand: true, cwd: '<%= dirs.src %>/'}
                ]
            }
        },

        concat: {
            css: {
                src: ['<%= dirs.src %>/css/normalize.css',
                      '<%= dirs.src %>/css/jquery.fancybox.css',
                      '<%= dirs.src %>/css/style.css'
                ],
                dest: '<%= dirs.dest %>/css/pack.css'
            },
            js: {
                src: ['<%= dirs.src %>/js/plugins.js',
                      '<%= dirs.src %>/js/jquery.scrollUp.js',
                      '<%= dirs.src %>/js/jquery.mousewheel.js',
                      '<%= dirs.src %>/js/jquery.fancybox.js'
                ],
                dest: '<%= dirs.dest %>/js/pack.js'
            }
        },

        uncss: {
            options: {
                ignore: [/(#|\.)fancybox(\-[a-zA-Z]+)?/, '#scrollUp'],
                htmlroot: '<%= dirs.dest %>',
                ignoreSheets: [/fonts.googleapis/]
            },
            dist: {
                src: '<%= dirs.dest %>/**/*.html',
                dest: '<%= concat.css.dest %>'
            }
        },

        cssmin: {
            dist: {
                options: {
                    keepSpecialComments: 0,
                    compatibility: 'ie8'
                },
                files: {
                    '<%= concat.css.dest %>': '<%= concat.css.dest %>'
                }
            }
        },

        uglify: {
            options: {
                compress: {
                    warnings: false
                },
                mangle: true,
                preserveComments: false
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
                    collapseWhitespace: true,
                    minifyJS: true,
                    removeComments: true
                },
                expand: true,
                cwd: '<%= dirs.dest %>',
                dest: '<%= dirs.dest %>',
                src: ['**/*.html']
            }
        },

        filerev: {
            css: {
                src: '<%= dirs.dest %>/css/**/{,*/}*.css'
             },
            js: {
                src: [
                    '<%= dirs.dest %>/js/**/{,*/}*.js',
                    '!<%= dirs.dest %>/js/jquery*.min.js'
                ]
            },
            images: {
                src: [
                    '<%= dirs.dest %>/img/**/*.{jpg,jpeg,gif,png}'
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

        connect: {
            options: {
                hostname: 'localhost',
                livereload: 35729,
                port: 8000
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
            files: ['<%= dirs.src %>/**', '.csslintrc', '.jshintrc', 'Gruntfile.js'],
            tasks: 'dev'
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

        validation: {
            options: {
                charset: 'utf-8',
                doctype: 'HTML5',
                failHard: true,
                reset: true
            },
            files: {
                src: '<%= dirs.dest %>/**/*.html'
            }
        }

    });

    // Load any grunt plugins found in package.json.
    require('load-grunt-tasks')(grunt, {scope: 'devDependencies'});
    require('time-grunt')(grunt);

    grunt.registerTask('dev', [
        'clean',
        'copy',
        'includereplace',
        'useminPrepare',
        'concat',
        'filerev',
        'usemin'
    ]);

    grunt.registerTask('build', [
        'clean',
        'copy',
        'includereplace',
        'useminPrepare',
        'concat',
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
        'validation'
    ]);

    grunt.registerTask('default', [
        'dev',
        'connect',
        'watch'
    ]);

};
