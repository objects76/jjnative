{
  "targets": [
    {
      "target_name": "utils",
      'type': 'static_library',
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc+": [ "-std=c++17" ],
      "sources": [ "<!@(ls -1 *.c*)"],
      'conditions': [
        ['OS!="mac"', {'sources/': [['exclude', '-mac\\.*']]}],
        ['OS!="win"', {'sources/': [['exclude', '-win32\\.*']]}],
      ],
      'include_dirs': ["<!(node -p \"require('node-addon-api').include\")"],
      'dependencies': [],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS", "STANDALONE" ],

      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions": [ "-std:c++17", ],
        },
      },
    }
  ]
}
