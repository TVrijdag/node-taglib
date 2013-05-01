{
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
              '<!@(taglib-config --cflags)'
            ]
          }
        }, {
          
        }],
        ['OS=="win"', {
          'libraries': ['C:/taglib/lib/tag.lib'],
          'defines': [
            '_WINDOWS',
            # to avoid problems with winsock2 inclusion in windows.h
            'WIN32_LEAN_AND_MEAN'
          ],
          'include_dirs': ['C:/taglib/include/taglib'],
         "copies":
         [
            {
               'destination': '<(module_root_dir)/build/Release',
               'files': ['C:/taglib/bin/tag.dll', 'C:/taglib/bin/zlib.dll', 'C:/taglib/bin/zlibd.dll']
            }
         ]
        }, { # OS!="win"
          'libraries': ['<!(taglib-config --libs)'],
          'cflags': [
            '<!@(taglib-config --cflags)'
          ]
        }]
      ]
    }
  ]
}
