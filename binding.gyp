{
  "conditions": [
    ['OS=="win"', {
      "variables": {
         "taglib%": "C:/taglib"
      }
    }, {
      "variables": {    
         "taglib%": "/usr"
      }
    }]
  ],
  "targets": [
    {
      "target_name": "taglib",
      "sources": ["src/bufferstream.cc", "src/tag.cc", "src/taglib.cc"],
      "include_dirs": ["<!(node -e \"require('nan')\")"],
      "conditions": [
        ['OS=="mac"', {
          # cflags on OS X are stupid and have to be defined like this
          # copied from libxmljs
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '<!@(<(taglib)/bin/taglib-config --cflags)'
            ]
          }
        }, {
          
        }],
        ['OS=="win"', {
          'libraries': ['<(taglib)/lib/tag.lib'],
          'defines': [
            '_WINDOWS',
            # to avoid problems with winsock2 inclusion in windows.h
            'WIN32_LEAN_AND_MEAN'
          ],
          'include_dirs': ['<(taglib)/include/taglib'],
          "copies": [
             {
                'destination': '<(module_root_dir)/build/Release',
                'files': ['<(taglib)/bin/tag.dll', '<(taglib)/bin/zlib.dll', '<(taglib)/bin/zlibd.dll']
             }
          ]
        }, { # OS!="win"
          'libraries': ['<!(<(taglib)/bin/taglib-config --libs)'],
          'cflags': [
            '<!@(<(taglib)/bin/taglib-config --cflags)'
          ]
        }]
      ]
    }
  ]
}
