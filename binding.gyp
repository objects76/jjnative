{
  "targets": [
    {
      "target_name": "jjnative",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc+": [ "-std=c++17" ],
      "sources": [ "native/*.cc", "native/*.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS', 'JJKIM' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [ '-std:c++17', ],
        },
      },
    }
  ]
}
