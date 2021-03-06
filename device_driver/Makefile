template.tex                                                                                        0000640 0124723 0035440 00000021454 12046115643 013234  0                                                                                                    ustar   herringr                        upg43475                                                                                                                                                                                                               \documentclass[letterpaper,10pt]{article}

\usepackage{graphicx}                                        
\usepackage{amssymb}                                         
\usepackage{amsmath}                                         
\usepackage{amsthm}                                          

\usepackage{alltt}                                           
\usepackage{float}
\usepackage{color}
\usepackage{url}

\usepackage{balance}
\usepackage[TABBOTCAP, tight]{subfigure}
\usepackage{enumitem}
\usepackage{pstricks, pst-node}


\usepackage{geometry}
\geometry{textheight=8.5in, textwidth=6in}

\newcommand{\cred}[1]{{\color{red}#1}}
\newcommand{\cblue}[1]{{\color{blue}#1}}

\usepackage{hyperref}
\usepackage{geometry}

\def\name{Ryley Herrington}
%\input{pygments.tex}

%% The following metadata will show up in the PDF properties
\hypersetup{
  colorlinks = true,
  urlcolor = black,
  pdfauthor = {\name},
  pdfkeywords = {cs411 ``kernel''},
  pdftitle = {CS 411: Ram Disk Driver and Crypto API},
  pdfsubject = {CS 411 Project 3},
  pdfpagemode = UseNone
}

\begin{document}
\begin{center}
{\large CS411 Project 3: RAM Disk Driver and Crypto API} \\ % \\ = new line
Ryley Herrington
November 5, 2012
\end{center}

\section{Introduction}
The goal of this project was to write a RAM Disk driver for the Linux Kernel which allocates a chunk of memory and presents it as a block device. Also, as an added bonus of complexity, we were supposed to use Linux Kernel's Crypto API, and add encryption to your block device such that your device driver encrypts and decrypts data when it is written and read.

\section{Design}
We knew what we had to do to allocate space in our current Hard Disk space and then treat is as a RAM area. We would do this, and then mount is as a filesystem (ext2), but we based it on the IO control that was found in the sbull implementation that's available inside the linux kernel. 

As far as the rest of the design goes for the RAM disk driver we based our whole implementation off of the sbull driver. But the encryption was more difficult. There are many explanation but our group write up was pretty clear on the whole subject (if I do say so myself). 

Basically, we will encrypt the data as it gets registered(queue'd) up to be written to the disk. We do this to get rid of the overhead of writing and encrypting all in the same method. We might encrypt som ething and read something from the disk, and this way we will always get it correct without having to worry about mutex locks.  

\section{Code Explanation}
The encryption algorithm selected was AES, using the ECB (Electronic Code Book) implementation. This is believed to be the simplest version of the AES encryption system.

We also chose to implement it in the request handler because we didn’t want to take the overhead hit of encrypting and writing to the device at the same time and because it was easier to handle the flow of control. 

If you look in the request method we check to see if we’re writing to or reading from the device, and depending on that value we will either encrypt then transfer the data to the writer method, or transfer back from the data then decrypt and read. Overall, we create a key, a seed value for the cryptographic algorithm, and the block cipher struct that we will use throughout the method. 

We allocate the block cipher, then we create the request structure, and if either of them fails we will return and deallocate anything we’ve already made. Otherwise we set the key for the cipher based on our seed value and then we get to the core. We will set the cipher and if we’re writing we will use the crypto asynchronous encrypt method, and if we’re reading we will decrypt it. Fairly simple to understand once it’s in place but figuring out that it needs to be asynchronous was difficult. And then, depending on the return value, we will return the encrypted/decrypted structure we want or fail out of the method. 

As far as the request method goes, we check to see if it's a non command operation. Then we allocate some area for the encrypted data using kmalloc (needed to be freed later as well). We will copy the data to be encrypted into a buffer, printk it out for testing purposes, and then transfer it to the disk. If we're reading instead of writing, we will transfer from the disk, and then decrypt it. 


The device driver is designed to automatically create a device in /dev/team80 upon initialization.  This device then requires that partitions be setup on it (which can be done with cfdisk) and a ﬁle system initialized on it. EXT2 via mkfs.ext2 provided a simple ﬁle system that worked well within the very small amount of space allocated for the disk. After a ﬁle system is created, the disk can be mounted to a directory like any other disk device.

The sbull driver was a lot easier to explain because there was PLENTY of documentation. We allocate the space, give it information to make sure that it knows what type of data it will be (i.e. not normal, because this is supposed to act like RAM). Then we handle the io by having regular methods (ioctl) etc. But the only semi difficult part of this was the MODULE parameters because we kept failing because we didn't have the correct license ("GPL"). But once we fixed that, everything was hunky dory. The getgeo method was important as well, because without it we couldn't partition our drive. 

\section{Testing}
We included some printk's to make sure that the data we were going to encrypt was in plain text,and then we would encrypt it. We would print out the first three bytes of the data to prove it was regular. Then we would encrypt it, and then we would print out the first three bytes again to make sure it was gibberish. 

As a counter to that, we would print out the first three bytes of an encrypted section and then decrypt it , then read the first three bytes again to make sure it's correct. 

We did test this with a sample program and it worked surprisingly well. Also, we copied a few programs in there that we knew would take a while to execute for kicks and giggles. Unsurprisingly the programs executed very quickly, even the very first time. So that was pretty cool to prove that we could mount something into fake RAM and have it act pretty similarly. 

\section{What I learned}
The crypto api in the linux kernel is not documented as well as anything else in the entire world. But I also learned that encryption is a delicate process that needs to be well thought out before implementation or else you'll overflow your disk space and you'll screw up your drive. But it's important to encrypt a lot of data so this was probably a good experience. 

Also, drivers aren't as hard as I was thinking they would be. They have some difficult areas, but overall they're pretty handy and can act correctly when implemented well. 

\section{Revision Control Log}

\hspace{\stretch{3}} 
------------------------------------------------------------------------


r28 | reids | 2012-11-05 19:59:32 -0800 (Mon, 05 Nov 2012) | 2 lines


comment cleanup


------------------------------------------------------------------------


r27 | reids | 2012-11-05 19:53:00 -0800 (Mon, 05 Nov 2012) | 2 lines


another makefile fix


------------------------------------------------------------------------


r26 | reids | 2012-11-05 19:49:46 -0800 (Mon, 05 Nov 2012) | 2 lines


adding makefile settings


------------------------------------------------------------------------


r25 | reids | 2012-11-05 19:44:52 -0800 (Mon, 05 Nov 2012) | 2 lines


latest config and team8 file


------------------------------------------------------------------------


r24 | reids | 2012-11-05 10:44:12 -0800 (Mon, 05 Nov 2012) | 1 line


fixing for loop


------------------------------------------------------------------------


r23 | herringr | 2012-11-05 10:25:35 -0800 (Mon, 05 Nov 2012) | 1 line


Added printk's to prove it's working.


------------------------------------------------------------------------


r22 | reids | 2012-11-04 21:11:27 -0800 (Sun, 04 Nov 2012) | 2 lines


cleaning up some comments and code


------------------------------------------------------------------------


r21 | herringr | 2012-11-04 17:14:33 -0800 (Sun, 04 Nov 2012) | 1 line


Encryption working.

------------------------------------------------------------------------

r20 | reids | 2012-11-01 14:39:39 -0700 (Thu, 01 Nov 2012) | 2 lines


these are the changes made that got the intial test driver working on the squidly


------------------------------------------------------------------------

r19 | reids | 2012-10-30 13:01:32 -0700 (Tue, 30 Oct 2012) | 2 lines


trying to add project3 again


------------------------------------------------------------------------

%\subsection{sstf-iosched.c}
%input the pygmentized output of mt19937ar.c, using a (hopefully) unique name
%this file only exists at compile time. Feel free to change that.
%\input{__sstf-iosched.c.tex}

\end{document}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    