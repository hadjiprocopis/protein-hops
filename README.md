# protein-hops

## Author: Andreas Hadjiprocpis (andreashad2@gmail.com)
## Institute of Cancer Research (Erler Lab, Linding Lab)

This work has started while I was working at the Institute
of Cancer Research (2009-2013) in the Erler and Linding Labs.

## What it does
Proteins are associated with other proteins, somehow.
STRING-DB (https://string-db.org) provides
a free, public database of functional associations
between proteins (for many species including human).

The data is of the form:
    A B 123
where A and B are proteins and 123 is the confidence
that they are functionally associated. Some of these
associations are transitive and some are not.

For humans (the 9606 files in string-db.org) there are
11353057 associations (not unique proteins but pairs of
associated proteins).

This program reads a STRING-DB file, with the format
A B 123
...
etc
...

Some files to get you started are:

```
9606.protein.actions.v10.5.txt
9606.protein.links.detailed.v10.5.txt
9606.protein.links.v10.5.txt
```

The above are for humans. Mouse code is 10090.
How to get these files? Simply go to STRING-DB (at https://string-db.org/cgi/download.pl)
and enter the species code at the search box (e.g. `9606` for humans).
Download the `actions`, `links.detailed` and `links` files. The names above
contain a version number (10.5). Which will be different in the future, as it
was different in the past.

For example, I can download one of the files using `wget` from the commandline
and save it to current dir as follows:

```
wget 'http://stringdb-static.org/download/protein.links.v10.5.txt.gz'
```

### BEWARE do not use `https` to download these huge files. It is a waste of resources but it is trendy for some so here is this warning. Unless of course you are worried someone wants to corrupt your proteins!

Once you have string-db files, the program
takes the names of two proteins (the 'from' and
the 'to') and a range of the number of hops.
It then proceeds to search the database as a Graph
and report all associations which start with 'from',
end with 'to' and have as many intermediates as the
number of hops specified.

For example, running the command
(please note that this will take a lot of time \- 16
mins in my computer but probably more in others.
If you want to try out something, keep reading on)

```
src/C/graphos_enquiry_for_STRING --input /data/DATASETS/STRING_DB/9606.protein.links.v10.5.txt --ifs ' ' --skip_header_lines 1 --max_num_hops 2 --search '9606.ENSP00000000233,9606.ENSP00000301744' --output out
```

will report all associations between 9606.ENSP00000000233 and 
9606.ENSP00000301744 with a maximum of two hops between them
(BEWARE it may take long time),
i.e.
```
9606.ENSP00000000233 9606.ENSP00000301744 (the direct one if it exists)
9606.ENSP00000000233 A 9606.ENSP00000301744 (1st degree, 1 intermediary protein 'A')
9606.ENSP00000000233 A B 9606.ENSP00000301744 (2nd degree, 2 intermediaries 'A' and 'B')
```

This is version 4.2 and it is a preliminary version
provided as a proof-of-concept for my idea.

Version 11.0 does many many more things.
It does search. Very well.

But it also builds a network of
interactions with the protein intermediaries too.
It clusters this network either wrt to function or
with say expression if specified.
It then assesses the statistical
significance of the clustering using monte carlo
simulation and t-test.

V11 can be provided upon request and subsequent interviewing.
If successful, the candidate user will be provided with V11
and supported.

## Author: Andreas Hadjiprocpis (andreashad2@gmail.com)
## Institute of Cancer Research (Erler Lab, Linding Lab)
Although the idea and implementation are mine, I used
ICR's resources.

# download:
I suggest you download the tarball from git repository at:

https://raw.githubusercontent.com/hadjiprocopis/protein-hops/master/grapher-4.2.tar.gz

(or find file ```grapher-4.2.tar.gz``` in my repository at https://github.com/hadjiprocopis)

# compile and install:
At a unix (OSX or Linux, please do not ask me question about windows)
command prompt do:
```
tar xvzf grapher-4.2.tar.gz
cd grapher-4.2
./configure --disable-debug
```
If configure likes your system, finds all
dependancies and is in a good day,
you may proceed to compilation:
```
make clean && make all
```

Then probably you have the executable at
```
src/C/graphos_enquiry_for_STRING
```

# running the program:
In order to get accustomed to the various command line options run:
```
src/C/graphos_enquiry_for_STRING --help
```
and study the output. Below I am giving you a small
test run with basic options.

Firstly you must download a STRING-DB protein
associations file, for example the one for
human species proteins at:
```
http://stringdb-static.org/download/protein.links.v10.5/9606.protein.links.v10.5.txt.gz
```

Note that I have changed 'https://...' to 'http://...'
Using https for public protein data is one way to destroy the world.

Then you need to uncompress it and store it somewhere,
let's say at the current dir
```
gunzip 9606.protein.links.v10.5.txt.gz
```

which will yield the file
```
9606.protein.links.v10.5.txt
```
in the current directory.

Now you are ready to run the grapher.

```
src/C/graphos_enquiry_for_STRING --input 9606.protein.links.v10.5.txt --ifs ' ' --skip_header_lines 1 --max_num_hops 1 --search '9606.ENSP00000000233,9606.ENSP00000301744' --output out.hops1
```

This will produce the following output:

```
graphos_enquiry_for_STRING : reading edges from file '/data/DATASETS/STRING_DB/9606.protein.links.v10.5.txt' ... 
read_file_in_memory : read 522240609 bytes from file '/data/DATASETS/STRING_DB/9606.protein.links.v10.5.txt' in memory.
done, read 11353057 lines (including header & empty), user: 5.170934, system: 0.276579 (5.170934, 0.278411)
graphos_enquiry_for_STRING : 1 of 1) recursive search 9606.ENSP00000000233 -> 9606.ENSP00000301744, min num hops: 1, max num hops: 1
graphos_enquiry_for_STRING : 1 of 1) found 818 paths with criteria specified, user: 0.509191, system: 0.000000 (5.680152, 0.278413)

graphos_enquiry_for_STRING : doing calculations on the paths ... user: 0.000075, system: 0.000000 (5.680241, 0.278413)

graphos_enquiry_for_STRING : 1 of 1) saving paths to files (out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.*) ...
1 hops yielded 818 paths, output file 'out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.1hops.txt'.
1 hops yielded 1636 edges, output file 'out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.edgelist.1hops.txt'.
all hops yielded 1636 edges, output file 'out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.edgelist.allhops.txt'.
1 hops yielded 820 nodes, output file 'out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.nodelist.1hops.txt'.
all hops yielded 820 nodes, output file 'out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.nodelist.allhops.txt'.
user: 0.003015, system: 0.000000 (5.683262, 0.278413)
graphos_enquiry_for_STRING : done, 0 (0.0%) searches failed, 1 (100.0%) searches succeeded, of a total of 1.
user: 6.039501, system: 0.276642 (6.039501, 0.278413)
```

Which says that it finished in about 6 seconds and the output is in the
current directory prefixed with 'out.hops1.'

The file
```
out.hops1.9606.ENSP00000000233.9606.ENSP00000301744.1hops.txt
```

contains entries like:
```
9606.ENSP00000000233()  ()      9606.ENSP00000358867()  ()      9606.ENSP00000301744()  (1)
```

which, as you can imagine is the association of the 'from' protein
(at the left) with the 'to' protein (at the right) and all
the intermediate proteins in the middle.

If you have a fast computer and a lot of memory then you can try
the 2nd degree associations, i.e. with 2 intermediaries (plus 1 'from'
and 1 'to' makes a chain of 4 proteins in total).

It will take exponentially more time to complete.
In my computer it took 16 minutes instead of
6 seconds.

Sometimes the program may run out of memory. So try more hops than
1 if you have confidence in your hardware. Otherwise, if all memory is
exhausted, the computer will become very slow and it may not be
easy to save any work you have been doing.

```
src/C/graphos_enquiry_for_STRING --input /data/DATASETS/STRING_DB/9606.protein.links.v10.5.txt --ifs ' ' --skip_header_lines 1 --max_num_hops 2 --search '9606.ENSP00000000233,9606.ENSP00000301744' --output out
```

So, before running the 2-hops, save all your work and be ready
to reboot your computer if it becomes too slow (usually having access
to a terminal will be sufficient to kill the program using:
```
killall graphos_enquiry_for_STRING
```

There are more enhancements which have been added in subsequent
versions which I have not released yet. Please contact me
if you are interested or you have ideas for enhancements
and additions.

## Dependencies
There are two dependencies which you can get from my repository.
One is XA_STATS and the other is PTHREADPOOL.

## Author: Andreas Hadjiprocpis (andreashad2@gmail.com)
## Institute of Cancer Research (Erler Lab, Linding Lab)
This work has started while I was working at the Institute
of Cancer Research (2009-2013).
