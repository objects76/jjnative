{
  "targets": [
    {
      "target_name": "jjnative",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc+": [ "-std=c++17" ],
      "sources": [ "<!@(ls -1 *.c*)",  "<!@(ls -1 *.mm)", "jjnative-win32.rc" ],
     
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", 'utils/'],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS", "_CRT_SECURE_NO_WARNINGS" ],

      'conditions': [
        ['OS!="mac"', {
          'sources/': [['exclude', '-mac\\.*']]
          }],
        ['OS!="win"', {
          'sources/': [['exclude', '-win32\\.*']],
          }],
        ['OS!="linux"', {
          'sources/': [['exclude', '-linux\\.*']],
          }],
      ],

      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions": [ "-std:c++17", ],
        },
        'VCLinkerTool': {
          'GenerateDebugInformation': 'true',
        },
      },

      #'dependencies': ['utils/utils.gyp:*'],
    }
  ]
}
