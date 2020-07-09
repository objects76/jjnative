{
  "targets": [
    {
      "target_name": "jjnative",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc+": [ "-std=c++17" ],
      "sources": [ '<!@(ls -1 native/*.c*)' ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [ '-std:c++17', ],
        },
      },
    }
  ]
}
