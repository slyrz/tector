# tector

## Build

First create the missing Makefile and configure script by running `autoreconf`,
then run the configure script.

	autoreconf --force --install
	./configure

Once you executed these commands, you can build the source by running

	make
	make check

## Usage

Tector comes with three programs: `filter`, `vocab` and `model`.

The `filter` program is just a handy tool to clean text. It performs
stemming and stopword removal. You can run filter by passing files
as arguments or piping text to its stdin. The cleaned text will be
written to stdout.

	filter < dirty.txt > clean.txt
	filter dirty01.txt dirty02.txt > clean.txt

The `vocab` program is used to create vocabularies. A vocabulary is
basically just a list of words and their frequency. A word that isn't part of
the vocabulary won't be recognized by the language model, so make sure
to create the vocabulary on a rich set of data.

The `model` program creates and trains the language model and computes the vector
representations of all words in your vocabulary. To outline the basic usage
of `vocab` and `model`, here's a simple example.

### Example

First create an empty directory that will contain the vocabulary
and language model files. This directory will be the first argument of
all subsequent `vocab` and `model` calls.

	mkdir example

Prepare to train a vocabulary by calling `vocab create`. Then train it on
input - let's say the directory text/ contains a lot of text files.

	vocab create example
	vocab train example text/*

That's it. You can call `vocab train` multiple times. To take a look at the
words in your vocabulary, run

	vocab print example

Now that the vocabulary is ready, create a language model by calling

	model create example

I need to squeeze a warning in here: right now the language
model can not deal with vocabulary changes. So if you want to re-train your
vocabulary, copy it to another directory and work on it there.

Back to the example. Just like vocabulary, the language model can be trained
by calling

	model train example text/*

And just like the `vocab train` command, `model train` can be called
multiple times.

	model train example more_text/*
	model train example even_more_text/*

After sufficient training, generate the word vectors by calling

	model generate example

That's it.

### License

tector is released under Apache License, Version 2.0.
You can find a copy of the Apache License in the [LICENSE](./LICENSE) file.
