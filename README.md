node-taglib
===========

node-taglib is a simple binding to
[TagLib](http://developer.kde.org/~wheeler/taglib/) in Javascript.

It requires [node.js](http://nodejs.org) and taglib header files (on Debian systems, install `libtag1-dev`).

node-taglib offers only an abstract interface without giving access to extended
file-specific attributes. It does allow custom resolvers though. Synchronous
write support is supported for Metadata.

**NOTE: Asynchronous API requires use of TagLib [from git][taglib-git] since
certain bugs present in the released v1.7 cause problems.**

[taglib-git]: https://github.com/taglib/taglib

## Example

```js
// load the library
const taglib = require('taglib');

// asynchronous API
taglib.read(path, (err, metadata) => {
    metadata.artist; // => "Queen"
    metadata.title = "Erm";
    metadata.saveSync(); // synchronous
});

// synchronous API
let metadata = taglib.readSync(path);

metadata.title; // => "Another one bites the dust"
metadata.artist; // => "Kween"
metadata.artist = "Queen";

metadata.saveSync(); // => true

// Or asynchronous
metadata.save((err) => {
});

// Close the metadata and clear reference
metadata.close(); // => true
metadata = undefined;
```

## Installation

### via npm (Recommended)

    npm install --save git://github.com/TVrijdag/node-taglib.git
    
    # or use another directory where taglib headers are located (default /usr or C:/taglib)
    npm install --save git://github.com/TVrijdag/node-taglib.git --taglib=/path/to/taglib

### From source

    # make sure you have node and taglib installed
    git clone git://github.com/TVrijdag/node-taglib.git
    cd node-taglib
    npm install .
    node examples/simple.js /path/to/mp3_or_ogg_file
    # you can now require('./taglib')

The `examples` show usage.

## API

### read(path, callback)
### read(buffer, format, callback)

The function you will most likely want to use. `callback` should have signature
`callback(err, metadata)`.

If there was an error reading the file, `err` will be non-null and `metadata`
will be `null`.  The error will be an object with field `code` having the 
integer error code (`errno.h`) and field `message` will have a string representation.

`metadata` will be a [Metadata](#metadata) object with the following tag information.
(node-taglib currently supports only the fields common to all formats):

* title   (string)
* album   (string)
* comment (string)
* artist  (string)
* track   (string)
* year    (integer)
* genre   (string)

In addition, the following audio properties may be available. All of them are:

* length      (integer)
* bitrate     (integer)
* sampleRate  (integer)
* channels    (integer)

These properties are undefined when the audio properties are not available
for the file or stream. Writing audio properties is not supported.

In the second variant, which can read from a buffer, `format` should be
a string as specified in [Formats](#formats).

### readSync(path)
### readSync(buffer, format)

Read the metadata from the file at `path` _synchronously_. Returns a `Metadata`. If
errors occurred, throws an error.

The error will be an object with field `code` having the integer error code
(`errno.h`) and field `message` will have a string representation.

In the second variant, which can read from a buffer, `format` should be
a string as specified in [Formats](#formats).

### Metadata

Result from `read()` and `readAsync()`. A Metadata object allows _read-write_ access to all the tag fields. All audio 
properties are read-only. For valid field names see `read()` above.

To get a value, simply access the field -- `metadata.artist`.

To set a value, assign a value to the field -- `metadata.year = 2012`. You **will
have to call `save()` or `saveSync()`** to actually save the changes to the file on disc.

##### Large number of files

Due to TagLib's design, every `Metadata` object in memory has to keep its backing
file descriptor open. If you are dealing with a large number of files, you will
soon run into problems because operating systems impose limits on how many
files a process can have open simultaneously. Use [Metadata.close()](#metadataclose) 
after the metadata is no longer needed.

### Metadata.close()

Clears the file descriptor such that the issue mentioned above can be prevented. Note 
that properties and methods of the closed metadata will no longer function after closing. 

```js
let metadata = taglib.readSync(path);
metadata.artist; // => "Kween"
metadata.artist = "Queen";
metadata.saveSync(); // => true
metadata.close(); // => true
metadata.artist; // => error
```

### Metadata.save(callback)

Save any changed tag fields in the Metadata object to disk _asynchronously_. 
`callback` will be invoked once the save is done, and should have a signature `(err)`.
`err` will be `null` if the save was successful, otherwise it will be an object with
`message` having the error string and `path` having the file path.

Metadata objects obtained from a buffer or those which are closed will throw an 
error if `save()` is called.

### Metadata.saveSync()

Save any changed tag fields in the Metadata object to disk _synchronously_. 
Throws an error if the save failed.

Metadata objects obtained from a buffer or those which are closed will throw an 
error if `saveSync()` is called.

### taglib.addResolvers(\[resolver1\[, resolver2\[, ...]]])

Adds JavaScript functions that will be called to resolve the filetype of
a file. Each resolver will be added to the front of the resolver queue. So the
last resolver will be called first. Multiple calls to `addResolvers` are
allowed.

Each resolver must be a JavaScript function which takes a `filename` parameter
and returns a format `string`. List of [formats](#formats).

### Formats {#formats}

Any place where `node-taglib` expects a format can be passed one of these
(case-insensitive):

    "MPEG"
    "OGG"      - Ogg Vorbis
    "OGG/FLAC" - Ogg FLAC
    "FLAC"
    "MPC"
    "WV"
    "SPX"      - Ogg Speex
    "TTA"
    "MP4"
    "ASF"
    "AIFF"     - RIFF AIFF
    "WAV"      - RIFF WAV
    "APE"
    "MOD"
    "S3M"
    "IT"
    "XM"

These correspond directly to the [filetypes
supported](http://developer.kde.org/~wheeler/taglib/api/classTagLib_1_1File.html)
by TagLib.  If the filetype cannot be determined, return anything other than
one of these literals.

Asynchronous resolvers (which indicate the filetype via a callback rather than
a return value) are not supported.

### taglib.WITH_ASF

A boolean representing whether node-taglib supports ASF files. Depends on
feature being enabled in TagLib.

### taglib.WITH_MP4

A boolean representing whether node-taglib supports MP4 files. Depends on
feature being enabled in TagLib.

Contributors are listed at: <https://github.com/nikhilm/node-taglib/contributors>
