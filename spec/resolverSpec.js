var assert = require('assert'),
  vows = require('vows'),
  fs = require('fs'),
  Taglib = require('../index');

if (Taglib.addResolvers) {

Taglib.addResolvers(function(fn) {
  var data = fs.readFileSync(fn, 'ascii');
  if (data.substring(0, 3) == 'ID3')
    return 'mpeg';
  return '';
});

vows.describe('taglib bindings')
  .addBatch({
    'opening UTF-8 Path': {
      topic: function() {
        return Taglib.readSync(__dirname + '/sample-with-ütf.mp3');
      },
      'should be a `Metadata`': function(metadata) {
        assert.equal('Metadata', metadata.constructor.name);
      }
    },

    'ASF support yes/no should be defined': {
      topic: Taglib.WITH_ASF,
      'is boolean': function(topic) {
        assert.isBoolean(topic);
      }
    },

    'MP4 support yes/no should be defined': {
      topic: Taglib.WITH_MP4,
      'is boolean': function(topic) {
        assert.isBoolean(topic);
      }
    },

    'reading Metadata from file': {
      'using readSync': {
        topic: function () {
          return Taglib.readSync(__dirname + '/sample.mp3');
        },
        'should be a `Metadata`': function (metadata) {
          assert.equal('Metadata', metadata.constructor.name);
        },
        'should be `A bit-bucket full of tags`': function (metadata) {
          assert.equal('A bit-bucket full of tags', metadata.title);
        },
        'should be a `A bit-bucket full of tags`': function (metadata) {
          assert.equal('A bit-bucket full of tags', metadata.title);
        },
        'should be by `gitzer\'s`': function (metadata) {
          assert.equal('gitzer\'s', metadata.artist);
        },
        'should be on `Waffles for free!`': function (metadata) {
          assert.equal("Waffles for free!", metadata.album);
        },
        'should be the first': function (metadata) {
          assert.equal(1, metadata.track)
        },
        'should be from 2011': function (metadata) {
          assert.equal(2011, metadata.year);
        },
        'should have a silly comment': function (metadata) {
          assert.equal("Salami Wiglet.", metadata.comment);
        },
        'should close Metadata': function (metadata) {
          metadata.close();
          assert.equal(undefined, metadata.artist);
        }
      },
      'using read': {
        topic: function() {
          var filename = __dirname + '/sample2.mp3';
          fs.writeFileSync(filename, fs.readFileSync(__dirname + '/sample.mp3'));
          Taglib.read(__dirname+'/sample2.mp3', this.callback);
        },
        'should be called with two arguments': function (err, metadata) {
          assert.isNull(err);
          assert.isObject(metadata);
        },

        'should be a `Metadata`': function(err, metadata) {
          assert.equal('Metadata', metadata.constructor.name);
        },
        'should be `A bit-bucket full of tags`': function (err, metadata) {
          assert.equal('A bit-bucket full of tags', metadata.title);
        },
        'should be a `A bit-bucket full of tags`': function (err, metadata) {
          assert.equal('A bit-bucket full of tags', metadata.title);
        },
        'should be by `gitzer\'s`': function (err, metadata) {
          assert.equal('gitzer\'s', metadata.artist);
        },
        'should be on `Waffles for free!`': function (err, metadata) {
          assert.equal("Waffles for free!", metadata.album);
        },
        'should be the first': function (err, metadata) {
          assert.equal(1, metadata.track)
        },
        'should be from 2011': function (err, metadata) {
          assert.equal(2011, metadata.year);
        },
        'should have a silly comment': function(err, metadata) {
          assert.equal("Salami Wiglet.", metadata.comment);
        },
        'should close Metadata': function(err, metadata) {
          metadata.close();
          assert.equal(undefined, metadata.artist);
        }
      },
    },

    'reading Metadata from non-existent file using readSync': {
      // nested functions because vows automatically calls a topic
      // function
      topic: function() {
        return function() {
          return Taglib.readSync('thisfileobviouslyshouldnot.exist');
        }
      },

      'should throw an exception': function(topic) {
        assert.throws(topic, /readable/);
      }
    },

    'reading Metadata from a non-audio file using readSync': {
      topic: function() {
        return function() {
          return Taglib.readSync(__filename);
        }
      },

      'should throw an exception': function(topic) {
        assert.throws(topic, /extract tags/);
      }
    },


    'reading Metadata from non-existent file using read': {
      // nested functions because vows automatically calls a topic
      // function
      topic: function() {
        Taglib.read('thisfileobviouslyshouldnot.exist', this.callback);
      },

      'should throw an exception': function(err, metadata) {
        assert.match(err.message, /readable/);
        assert.isNull(metadata);
      }
    },

    'reading Metadata from a non-audio file using read': {
      topic: function() {
        Taglib.read(__filename, this.callback);
      },

      'should throw an exception': function(err, metadata) {
        assert.match(err.message, /extract tags/);
        assert.isNull(metadata);
      }
    },

    'reading audio properties': {
      'using readSync': {
        topic: function () {
          return Taglib.readSync(__dirname + '/blip.mp3');
        },
        'should have length 1 second': function(metadata) {
          assert.equal(metadata.length, 1);
        },
        'should have bitrate 128kbps': function(metadata) {
          assert.equal(metadata.bitrate, 128);
        },
        'should have sampleRate 44100Hz': function(metadata) {
          assert.equal(metadata.sampleRate, 44100);
        },
        'should have 2 channels': function(metadata) {
          assert.equal(metadata.channels, 2);
        },
      },

      'using read': {
        topic: function () {
          var filename = __dirname + '/blip2.mp3';
          fs.writeFileSync(filename, fs.readFileSync(__dirname + '/blip.mp3'));
          Taglib.read(__dirname + '/blip2.mp3', this.callback);
        },
        'should have length 1 second': function(err, metadata) {
          assert.equal(metadata.length, 1);
        },
        'should have bitrate 128kbps': function(err, metadata) {
          assert.equal(metadata.bitrate, 128);
        },
        'should have sampleRate 44100Hz': function(err, metadata) {
          assert.equal(metadata.sampleRate, 44100);
        },
        'should have 2 channels': function(err, metadata) {
          assert.equal(metadata.channels, 2);
        },
      },
    },

    'writing Metadata to File': {
      topic: function() {
        var filename = __dirname+'/sample-write.mp3';
        fs.writeFileSync(filename, fs.readFileSync(__dirname+'/sample.mp3'));
        var t = Taglib.readSync(filename);
        t.title = 'Something completely different…';
        t.saveSync();
        t.close();
        return filename;
      },
      'should have written `Something completely different…` to title': function (filename) {
        var tag = Taglib.readSync(filename);
        assert.equal(tag.title, "Something completely different…");
        tag.close();
      }
    },

    'stripping Metadata from File': {
      topic: function() {
        var filename, t;
        filename = __dirname + '/sample-clean.mp3';
        fs.writeFileSync(filename, fs.readFileSync(__dirname + '/sample.mp3'));
        t = Taglib.readSync(filename);
        t.title = null;
        t.artist = null;
        t.album = null;
        t.genre = null;
        t.year = null;
        t.comment = null;
        t.track = null;
        t.saveSync();
        t.close();
        t = undefined;
        return filename;
      },
      'should result in a Metadata that `isEmpty`': function(filename) {
        var tag = Taglib.readSync(filename);
        assert.ok(tag.isEmpty());
      }
    },

    'writing Metadata to a file asynchronously': {
      topic: function() {
        var filename = __dirname+'/sample-write-async.mp3';
        fs.writeFileSync(filename, fs.readFileSync(__dirname+'/sample.mp3'));
        var self = this;
        Taglib.read(filename, function(err, metadata) {
          if (err) {
            self.callback(err);
          }
          metadata.title = 'Something completely different...';
          metadata.save(function(err) {
            metadata.close();
            metadata = undefined;
            self.callback(err, filename);
          });
        });
      },
      'should have written `Something completely different...` to title': function (filename) {
        var metadata = Taglib.readSync(filename);
        assert.equal(metadata.title, "Something completely different...");
        metadata.close();
      }
    },

  }).export(module);
}
