**Developing compression algorithms with evolutionary algorithms**

**Giancarlo A.P.I. Catalano**

Student ID: 2822325

Supervisor: David Cairns

_October 2022_

# Introduction

This project revolves around the development of a data compression algorithm, characterized by the use of Evolutionary algorithms. Data compression aims to reduce the space occupied by data, which enables not only more files to be stored but also faster transmission rates across networks.

This is particularly important nowadays with the amount of data present on the internet, which gets sent back and forth constantly.

There is a multitude of compression algorithms, but each is specialized for a specific type of data (e.g. audio, images, text), but there isn't one that works well on every data type. [1]

The problem lies in choosing the right algorithm: given a file, which technique will yield the best result?

It may even be that the file is heterogeneous and would benefit from being split into segments to be compressed in different ways, which past research has shown to often be true [2-3] .

This dissertation is about applying evolutionary algorithms and other modern techniques in order to apply those compression algorithms to their full power and for the user to have more control over the quality of the compression size versus performance trade-off.

In particular, it aims to address the limitations of traditional compression algorithms in relation to more complex data types, which may contain many kinds of complex data within them.

##



## 1.1 Background and Context

As the modern world relies more and more on the accumulation and storage of data, compression has proven to be incredibly useful. Making data smaller has the obvious advantage of allowing for more storage, but also to send it more quickly over networks, which is a fundamental component of modern computing.

Compression algorithms are designed to analyze files and produce smaller versions of them. This is possible thanks to **redundancies** in the file at hand: patterns are detected and expressed in a more concise and dense manner.

Each compression technique is designed with certain circumstances in mind (the type of data, the origin of it, etc..), and works best when applied to its intended target.

### 1.1.1 Finding the right one is not so easy

For each data type, there are many techniques to be applied, and each has its strengths and weaknesses. Some may compress well but require lots of computation power, while others produce decent results very quickly.

Applying the wrong compression can lead to sub-par results, so it's essential to have some knowledge about the data. For example, photographs will be compressed well by JPEG, but for graphics PNG is far better.

Still, there are so many approaches and many settings, and it's impossible to know which will produce the results we want unless we try them all.

This is unfeasible: compressing a file is not a quick operation, and the amount of combinations to be tried increases very quickly.

Furthermore, a given file may contain lots of different kinds of data in it, think of PDFs, Word Documents, Videos, Webpages, Games: it's obvious that applying one algorithm to the entire file is likely to cause suboptimal results. It would be ideal to have a system that is able to analyze the file at hand, and obtain a " **recipe**" for the compression procedure, but this is not a simple task.

###


### 1.1.2 Evolutionary algorithms are just what we need

**Evolutionary algorithms** are ideal for tasks where we need to find the right solution, and trying all the possible answers is not feasible [4-5].

This is possible thanks to smarter ways of searching for solutions, inspired by **natural evolution**.

In nature, Darwinian evolution produces organisms which are well suited to their environment, and the same process is implemented by Evolutionary computation to produce solutions which perform well for a given task [4].

There are many existing examples of evolutionary techniques applied for compression, though these tend to be tailored for very specific contexts, such as scanned documents [6] , biomedical measurements [7-8], fractals [9] or text strings [10].

More importantly, there are some existing projects which are more akin to this, and they will be explored in the following sections.

## 1.2 Problem analysis

There are some issues that need to be addressed: simply feeding compression algorithms to an evolutionary simulation will not produce a usable program.

This is because the evolutionary process's performance depends on how we evaluate the solutions, which may be a very slow process.

In this case, a solution is a recipe for compression, and it includes which algorithms to apply and to what parts of the file they need to be applied on. To assess how well a "recipe" is performing, we look at how much it was able to compress the given data.

This is where the main problem lies: assessing the performance of a solution is very slow, because it involves applying many complex algorithms.

If the evolution process is faster, this will allow for more "evolution" to happen, and better "recipes" will be obtained. This means that if the solutions are designed with assessment speed in mind, the evolution process will yield better solutions.

But this is not trivial, and this dissertation will design a system that can decide good compressions by having a robust evolution system.

###


# 2 State of the Art

## 2.1 The state of Compression

Compression is used everywhere: many file formats are disguised zip files (.docx, .pptx, .msi, .jar), and many network protocols make use of them for every message sent over (especially gzip and Deflate).

The same algorithms get used for most compression procedures, especially LZ (Lempel-Ziv) techniques and PPM (Prediction by Partial Matching).

There is no clear winner: each has its pros and cons, but some simple criteria are the compression ability and speed.

These algorithms tend to be very complex and refined, and produce good compression at impressive speeds.

However, there is a lack of approaches which try to use multiple techniques throughout the same file, or that acknowledge that a lot of the files being compressed are not of the traditional kind: what happens when you compress an already compressed file for example? Such challenges are often ignored as the current approaches are "good enough", but there definitely is more to be compressed!

### 2.2.1 Divide and conquer for faster processing, but also better results

Many previous studies on the application of metaheuristic methods in compression came to the same initial step: divide the given file into many smaller segments [11-15] .

Each of the segments can then have a separate compression applied to it, which has many advantages:

- Parallel computation: as the segments are independent of each other, both the evolutionary component and the compression component can be parallelized
- Better compression: applying the right algorithms in the right places will produce better results than a generic algorithm applied for the entire file [16]

- Selective decompression: if the user only needs part of the file (e.g. files embedded within it), we don't have to decompress the entire file, but just the segments which contain it.

_The given data is divided in segments, each compressed separately with algorithms such as LZW (Lempel-Ziv-Welch), AC (Arithmetic Coding), RLE (Run Length Encoding).._

_This is in GP-zip, where an individual is a recipe for the entire file [11]_

### 2.2.3 Actually finding the best compression

Given a segment of data, how will the program produce a good compression recipe?

Here are shows two examples of how past studies approached this problem.

In _GP-zip2_, The segment to be compressed will be matched against a database of tried compressions, by using **Genetic Programming** to analyze the data. The dataset is likely to have encountered a similar segment, and knows that a certain recipe worked well for it, so that's the recipe that will be used. [3]

_W. H. Hsu and A. E. Zwarico_ developed a system which analyses the start, middle and end of files, and matches them against a table of datatypes, where for each entry is paired with its most appropriate compression. [13]

Having said that, the Genetic Programming solution was shown to be better than existing algorithms, but with the problem of being much slower, taking about 10 minutes to compress small files.

This a problem that affects all similar works: although the results are comparable to traditional algorithms, they are still slow. The causes will be discussed in more detail in the following sections.

## 2.2 Similar work

Compression in general is not an easy task: it has been shown long ago that it's impossible to make a compression algorithm that always compresses data , and that less than 1% of all files can be compressed lossless by one byte. [17]

The simplest way to compress data efficiently is to specialize on certain data formats, so that only the important redundancies have to be accounted for [17].

There are some popular general-purpose compression programs, like WinZip and 7-Zip, using the algorithms that are known to work well, in general, the very traditional techniques developed 2 decades ago.

### 2.2.1 The GP-zip family

The system that's most similar to this project is GP-zip3, which went through many iterations: GP-zip , GP-zip\*, GP-zip2, GP-zip3.

The system started out as a simple program that finds good compression recipes for programs through Genetic Programming, and simply applies those compressions.

This was GP-zip\*, which had the issue of being extremely slow during compression. This was addressed with Gp-zip2, which used Genetic Programming to create faster and approximated versions of slow GP-zip\* components.

GP-zip2 was much faster than its predecessor, but required a lot of training for its components (13 hours), and thus GP-zip3 was born with the intent of speeding up the training process while maintaining the quality of the original GP-zip\*.

GP-zip3 achieved the goals it set itself, and its compression capabilities were then adapted for the analysis of EMG signals (Muscle Contractions). [11]

In general, the GP-zip family is a strong indication that metaheuristic methods are a viable approach to data compression, especially thanks to their flexibility and adaptability

While GP-zip3 achieved good compression ratios, it still took 12-15 minutes to compress archives that were in the order of Megabytes long, and that's the greatest issue preventing these approaches from being explored more thoroughly. [16]

This is caused by the extensive use of complex and slow data structures:

- Evolutionary processes
- GP trees, used multiple times in every step of the process
- Statistical analysis of segments, including complex variables such as Kurtosis
- K-means Clustering algorithms, both building and matching against

While this dissertation will borrow a lot of ideas from the GP-zip family, the performance and complexity issues will be addressed early in the project, as this will allow for the project to be directed in different directions as needed as the dissertation progresses

### 2.2.2 Evolving preprocessors, by Parent & Nowe

In the studies shown so far, the systems mostly focused on applying algorithms and hoping for the best, but it is also important to preprocess the data to be compressed.

A lot of the redundancies present in data need to be "exposed" through preprocessing in order to get detected by the compressor.

Traditional algorithms do this to a small extent, but this part of compression is definitely not as researched as the compression algorithms themselves.

In [12], the system aimed to increase the **entropy** of the given data by applying transformations, which will cause the compressed file to be smaller.

Interestingly enough, the system constructed preprocessors that included conditional logic, and often checked properties about the data to decide what to do with it.

[7] shares an issue with the GP-zip family: it's overly complex.

In particular, the data is processed and compressed according to LISP-like S-expressions that are to be executed by a virtual machine, and they even take arguments.

The "compression recipes" evolved by the algorithm were quite complex and lead to very long running times and all the measures taken for all the components to be compatible with each other

Surprisingly, the speed of the program is not a problem: this is thanks to many optimizations and parallel computation.

There is a lot of elements that can be simplified away, while keeping the structure the same and produce similar results:

- Preprocessing instructions don't need to be S-expression, they could just be sequences of transformations
- A single data transformation can be simpler, and shouldn't have special requirements on the data (arguments and preconditions)
- No need for a Virtual machine if the instructions are simple enough.

### 2.2.3 Current Prototype

In the span of 4 weeks, a working prototype has been built, and it is able to compress and decompress files successfully at feasible execution speeds.

It is able to compress most files, though it performs best with simpler file formats (text, simple images, HTML pages).

This prototype currently includes the following features

- Compression and decompression is consistent
- Compression solutions are evolved for separate segments
- Many preprocessors and compression schemes are available

As more compression schemes are added, it is expected that the ability to compress files will improve significantly.

The system provides a modular structure to allow for fast prototyping. An example is the implementation of the **Fast Evolution Algorithm [18] ,** which has greatly improved the execution speed and lays the groundwork for future similar improvements.

A strong advantage of evolutionary algorithms is that they've been thoroughly researched, and a lot of useful algorithms can be implemented easily to improve the system as a whole.

The prototype is available on GitHub for inspection [19] .

#


# 3 Evaluation plan

## 3.1 What counts as a success?

There are various goals to be juggled to produce a satisfactory solution:

- Good compression sizes
- Good compression speed

Other criteria to be kept in mind, especially due to this being a Computer Science project:

- Flexible algorithms that can work on more than just files
- Solution that requires low effort to be expanded

The main goal is definitely the compression size: it would be very inconclusive to make a compression system that does some complicated process very quickly only to produce files that are barely smaller than the original.

Considering that the standard compression algorithm in use (zip, WinRAR, 7-zip) tends to produce files that are between 30% and 70% of the original in most cases, it would be satisfactory if this approach produced files that were 85% of the original size.

As for speed, the system is going to be compared to its most similar competitor, GP-zip3, and the standard algorithms: WinZip, 7-Zip, gzip and Deflate. If the speed of the produced program is comparable to that of the popular algorithms, being less than 10 times slower would be a success considering how much effort has been put into making those algorithms fast.

##


## 3.2 Functionality over features

A factor that has to be taken into consideration is development time available, as there are a lot of components that can be tuned and improved for speed, memory efficiency or code quality

- Preprocessors and compression
- Evolutionary framework
- Evolutionary settings
- How the data points are to be processed to make a decision component

And all of the above can be particularly time consuming, and they have lower priority compared to making the overall system work. Thus, in case of time constraints, the project can at least be said to be functional.

## 3.2 Measurable performance

In short, these metrics will be measured across many files types and sizes, and compared against the major compression algorithms

- Compression ratio

- Compression speed
- Consistency across runs (as the process is stochastic)

Then, underperforming areas will be investigated, to see if there are certain file types that tend to produce worse results.

The test data will be publicly available archives, which have been used in many compression related studies. These archives are the Calgary Corpus [20], the Canterbury Corpus [21] and the Silesia Corpus [22].

Thanks to the ESG evaluation framework available, the tests will have plenty of data to be analyzed, which will allow for accurate results obtainable in good time.

Additionally, the dataset will also be inspected to check if certain files are more suited for the system, for example biological samples, medical imaging or physical data. These are often the topic of compression schemes which are developed for specific fields, and it has been the direction taken by the GP-zip family [11]

#


# 4 Project Plan

## 4.1 Autumn

| **Week** | **Project Task** | **Deliverables** |
| --- | --- | --- |
| **Autumn: 7-8** | Implement more preprocessors and compressions available Asses the initial performance of each compression,and improve the compression and the speed where needed | Amount of possible ways in whichthe segment can be applied | 
| **Autumn: 9-10** | Implement more evolutionary methods, and a simple datalogging system | Amount of evolutionary methodsthat can be used.Logging system is stable |
| **Autumn: 11-12** | Implement a system that takes the data points andproduces the component that decides the preprocessors andcompressions to be applied. Interim code demonstration, which may include the decider ifavailable at that time | A simple "decider" that is usable forthe overall system. A system that is able to independently compress files using evolution |
| **Autumn: 13-15** | Code review but no new significant code added, due to exams and holidays Rough reading of papers related to the project | Code adheres to C++ best practices |

##


## 4.2 Spring

| **Week** | **Project Task** | **Deliverables** |
| --- | --- | --- |
| **Pre-spring** | Start familiarizing with ESG and the university computercluster, Implement unit tests to make sure the system doesn't have major faults | Tests to see if simplified versions of thesystem can be run on the cluster andon ESG, An initial dataset is produced, Most unit tests are passed|
| **Spring 1-2** | Keep learning ESG if needed. Implement a more mature approach that bakes the produceddata set into a decision component, e.g. a decision tree | The produced dataset can be passedthrough this component to make a morecomplete decision maker |
| **Spring 3-4** | Get feedback from peers and draft about outline, revisit whatwill be the start of the dissertation. Check which areas of the prototype need improvementin general | An improved and more comprehensiveoutline, especially when comparedwith how the prototype developed so far |
| **Spring 5-6** | Add more unit tests and small fixes where needed. Draft dissertation structure, using the updated outline asreference | Code passes all unit tests. Code can be run consistently enoughfor demonstrations. Initial structure of dissertation is ready |
| **Spring 7-8** | Benchmark system on university cluster. Expand technical sections of the dissertation. Prepare for poster presentation | Benchmarking data is available. Structure of presentation is ready |
| **Spring 9-10** | Complete poster presentation material. Analyze data obtained from benchmarking. Expand methodology section in dissertation | Poster presentation file is ready. Statistical analysis of benchmarkincluded in draft dissertation |
| **Spring 11** | Draft dissertation's evaluation and conclusion sections. Improve analysis of benchmarking data. Submit draft to supervisor for feedback | Dissertation word count should be atleast 6000 words |
| **Spring 12-13** | Revise draft using feedback from supervisor. Peer review among computer science students. Final code demonstration | Draft has at least 11000 words,feedback does not include major issuesabout the structure. |

#


# References

[1] A. Gupta, A. Bansal and V. Khanduja, "Modern lossless compression techniques: Review, comparison and analysis," in 2017, pp. 1-8.

[2] F. Ullah and K. M. Yahya, "A new data compression technique using an evolutionary programming approach," in _Emerging Trends and Applications in Information Communication,_ 2012, pp. 524-531.

[3] A. Kattan and R. Poli, "Evolution of human-competitive lossless compression algorithms with GP-zip2," _Genetic Programming and Evolvable Machines,_ vol. 12, _(4),_ pp. 335-364, 2011. Available: https://doi.org/10.1007/s10710-011-9133-6. DOI: 10.1007/s10710-011-9133-6.

[4] Marshall Brain, "How Digital Television Works," Available: https://electronics.howstuffworks.com/dtv4.htm.

[5] P. A. Vikhar, "Evolutionary algorithms: A critical review and its future prospects," in 2016, . DOI: 10.1109/ICGTSPICC.2016.7955308.

[6] V. Aysha, K. Balakrishnan and S. Sundar, "Parallel genetic algorithm for document image compression optimization," _2010 International Conference on Electronics and Information Engineering,_ vol. 2, pp. 2-487, 2010.

[7] K. Geetha _et al_, "An evolutionary lion optimization algorithm-based image compression technique for biomedical applications," _Expert Systems,_ vol. 38, _(1),_ pp. e12508, 2021. Available: https://onlinelibrary.wiley.com/doi/abs/10.1111/exsy.12508. DOI: 10.1111/exsy.12508.

[8] K. M. Hosny, A. M. Khalid and E. R. Mohamed, "Efficient compression of volumetric medical images using Legendre moments and differential evolution," _Soft Computing,_ vol. 24, _(1),_ pp. 409-427, 2020. Available: https://doi.org/10.1007/s00500-019-03922-7. DOI: 10.1007/s00500-019-03922-7.

[9] D. Saupe and M. Ruhl, "Evolutionary fractal image compression," in Sep. 1996, . DOI: 10.1109/ICIP.1996.559449.

[10] G. Trautteur, "A Kolmogorov complexity based genetic programming tool for string …," _Proceedings of the …,_ Available: https://www.academia.edu/60214445/A\_Kolmogorov\_complexitybased\_genetic\_programming\_tool\_for\_string\_.

[11] A. Kattan, "GP-zip Family," _Evolutionary Synthesis of Lossless Compression Algorithms: The GP-Zip Family,_ 2010.

[12] J. Parent and A. Nowe, "Evolving compression preprocessors with genetic programming," in _Proceedings of the 4th Annual Conference on Genetic and Evolutionary Computation,_ 2002, .

[13] W. H. Hsu and A. E. Zwarico, "Automatic synthesis of compression techniques for heterogeneous files," _Software: Practice and Experience,_ vol. 25, _(10),_ pp. 1097-1116, 1995. Available: https://onlinelibrary.wiley.com/doi/abs/10.1002/spe.4380251003. DOI: 10.1002/spe.4380251003.

[14] A. G. B. Tettamanzi, "Evolutionary Synthesis of a Fuzzy Image Compression Algorithm," _… Of the Fourth European Congress on …,_ Available: https://www.academia.edu/698598/Evolutionary\_Synthesis\_of\_a\_Fuzzy\_Image\_Compression\_Algorithm.

[15] T. M. Wolfley, _Genetic Programming for Image Compression._ University of California, Los Angeles, 2010.

[16] A. Kattan and R. Poli, "Evolutionary synthesis of lossless compression algorithms with GP-zip3," in July 2010, pp. 1-8.

[17] I. M. Pu, _Fundamental Data Compression._ 2005Available: https://books.google.co.uk/books?id=Nyt0HgC81I4C.

[18] M. Salami and T. Hendtlass, "A fast evaluation strategy for evolutionary algorithms," _Applied Soft_ _Computing,_ vol. 2, _(3),_ pp. 156-173, 2003. Available: https://www.sciencedirect.com/science/article/pii/S1568494602000674. DOI: 10.1016/S1568-4946(02)00067-4.

[19] (https://github.com/Giancarlo-Catalano/EvoCom). _GitHub repository of Prototype_.

[20] Anonymous "Calgary corpus," -08-04T09:14:56Z, 2022.

[21] Tim Bell _et al_, "The Canterbury corpus," _Http://Corpus.Canterbury.Ac.Nz,_ Available: https://cir.nii.ac.jp/crid/1571980075203777152.

[22] (Sebastian Deorowicz). _Silesia Corpus_. Available: https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia.
