{
  "targets": [
    {
      "target_name": "jjnative",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc+": [ "-std=c++17" ],
      "sources": [ "<!@(ls -1 *.c*)", "jjnative-win32.rc" ],
     
      'include_dirs': ["<!(node -p \"require('node-addon-api').include\")", 'utils/'],
      'dependencies': ['utils/utils.gyp:*'],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],

      'conditions': [
        ['OS!="mac"', {
          'sources/': [['exclude', '-mac\\.*']]
          }],
        ['OS!="win"', {
          'sources/': [['exclude', '-win32\\.*']],
          'defines': ['WIN32'],
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
    }
  ]
}
