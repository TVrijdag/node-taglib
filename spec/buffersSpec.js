var assert = require('assert'),
    vows = require('vows'),
    fs = require('fs'),
    Taglib = require('../index')

vows.describe('taglib bindings: Buffers')
.addBatch({
  'read metadata from mp3 buffer': {
    topic: function() {
      var buf = fs.readFileSync(__dirname+'/sample.mp3');
      Taglib.read(buf, 'mpeg', this.callback);
    },

    'should be called with two arguments': function (err, tag) {
      assert.equal(arguments.length, 2);
      assert.isNull(err);
      assert.isObject(tag);
    },

    'reading tags': {
      topic: function() {
        var buf = fs.readFileSync(__dirname+'/sample.mp3');
        Taglib.read(buf, 'mpeg', this.callback);
      },

      'title should be `A bit-bucket full of tags`': function (metadata) {
        assert.equal(metadata.title, 'A bit-bucket full of tags');
      },
      'artist should be by `gitzer\'s`': function (metadata) {
        assert.equal(metadata.artist, 'gitzer\'s');
      },
      'album should be on `Waffles for free!`': function (metadata) {
        assert.equal(metadata.album, "Waffles for free!");
      },
      'track should be the first': function (metadata) {
        assert.equal(metadata.track, 1);
      },
      'should be from 2011': function (metadata) {
        assert.equal(metadata.year, 2011);
      },
      'should have a silly comment': function(metadata) {
        assert.equal(metadata.comment, "Salami Wiglet.");
      }
    },

    'reading audioProperties': {
      topic: function() {
        var buf = fs.readFileSync(__dirname+'/blip.mp3');
        Taglib.read(buf, 'mpeg', this.callback);
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
      }
    },
  },

  'reading metadata from a buffer with unknown format': {
    topic: function() {
      var buf = fs.readFileSync(__dirname+'/sample.mp3');
      Taglib.read(buf, '', this.callback);
    },

    'should raise an error': function(err, _, _) {
      assert.isNotNull(err);
      assert.match(err.message, /Unknown file format/);
    }
  },

  'reading metadata from a buffer with wrong format': {
    topic: function() {
      var buf = fs.readFileSync(__dirname+'/sample.mp3');
      Taglib.read(buf, 'ogg', this.callback);
    },

    'should raise an error': function(err, _, _) {
      assert.isNotNull(err);
      assert.match(err.message, /Failed to extract tags/);
    }
  },

  'reading metadata from empty buffer': {
    topic: function() {
      var buf = Buffer.alloc(0);
      Taglib.read(buf, 'mpeg', this.callback);
    },

    'should raise an error': function(err, _, _) {
      assert.isNotNull(err);
      assert.match(err.message, /Failed to extract tags/);
    }
  },


  'readSync metadata from mp3 buffer': {
    topic: function() {
      var buf = fs.readFileSync(__dirname+'/sample.mp3');
      return Taglib.readSync(buf, 'mpeg');
    },

    'title should be `A bit-bucket full of tags`': function (tag) {
      assert.equal(tag.title, 'A bit-bucket full of tags');
    },
    'artist should be by `gitzer\'s`': function (tag) {
      assert.equal(tag.artist, 'gitzer\'s');
    },
    'album should be on `Waffles for free!`': function (tag) {
      assert.equal(tag.album, "Waffles for free!");
    },
    'track should be the first': function (tag) {
      assert.equal(tag.track, 1);
    },
    'should be from 2011': function (tag) {
      assert.equal(tag.year, 2011);
    },
    'should have a silly comment': function(tag) {
      assert.equal(tag.comment, "Salami Wiglet.");
    }
  },

  'readSync data from a buffer with unknown format': {
    topic: function() {
      return function() {
        var buf = fs.readFileSync(__dirname+'/sample.mp3');
        return Taglib.readSync(buf, '', this.callback);
      }
    },

    'should raise an error': function(topic) {
      assert.throws(topic, /Unknown file format/);
    }
  },

  'readSync data from a buffer with wrong format': {
    topic: function() {
      return function() {
        var buf = fs.readFileSync(__dirname+'/sample.mp3');
        return Taglib.readSync(buf, 'ogg');
      }
    },

    'should raise an error': function(topic) {
      assert.throws(topic, /extract tags/);
    }
  },

  'readSync data from empty buffer': {
    topic: function() {
      return function() {
         var buf = Buffer.alloc(0);
         return Taglib.readSync(buf, 'mpeg');
      }
    },

    'should raise an error': function(topic) {
      assert.throws(topic, /extract tags/);
    }
  },

  'writing to a tag from a buffer': {
    topic: function() {
      return function() {
        var buf = fs.readFileSync(__dirname+'/sample.mp3');
        var tag = Taglib.readSync(buf, 'mpeg');
        tag.artist = 'nsm';
        tag.saveSync();
      }
    },

    'should fail': function(topic) {
      assert.throws(topic);
    }
  }
}).export(module);
