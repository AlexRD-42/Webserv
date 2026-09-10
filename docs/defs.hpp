#pragma once
#include "core.hpp"

/* ============================================================================
	If we continue the analogies of words and pages, then:

	Char:		  1  B
	Word:		  8	 B
	Line:		 64	 B
	Pages:		  4	KB										64 lines
	Chapter:	256 KB						64 pages		4096 lines
	Book:		 16	MB		64 chapters		4096 pages		262144 lines

	Volume? Collection? Sigil? rune? link
============================================================================ */

/*
	If an allocator contained several pages, ideally you'd place each allocation at the
	start of a page, so that in the event that it grows higher than a page, mmap can
	reassign pages for its growth

	Let a macroblock contain 64 pages, or 256kb (indexed by a word)

	In the event that a single vector grows beyond page size, its growth can be handled
	by changing virtual mappings, because now every capacity increase is a integer multiple
	of 4096. It becomes literally a matter of just searching for free pages, and remapping

	However for smaller allocations (which aren't ideal regardless), this would quickly fill pages
	Ideally you'd want one macroblock that contains big allocations, and another that contains smaller ones

	The optimal use of space would be to correctly guess the allocations that grow a lot, and this could be achieved with
	an optional parameter that hints the expected size in conjunction with the initial requested size

	So for example, let's say we have a big macroblock (A) and a small macroblock (B)

	A) 
	Alloc1: 0		(Page 0)
	Alloc2: 4096	(Page 1)
	Alloc3: 8192	(Page 2)

	B)
	Alloc1: 0		(Page 0)
	Alloc2: 2048	(Page 0)
	Alloc3: 1024	(Page 0)
	Alloc4: 3072	(Page 0)

	It doesn't even have to be like this:

	A strong allocation (hinted or high initial alloc) would search for an empty page;

	A weak allocation would search for a page with the highest subdivision below the threshold.
	For example, page 5 contains one weak allocation and its potential growth would be 4096 and 
	effectively	unbounded due to virtual memory remapping. However now this allocation will be 
	limited to 2048, because this new weak allocation will occupy its space.

	This page would've had a subdivision of 1, and now it is 2 (this doesn't even have to be stored, it can be calculated)
	When a weak allocation searches for a place, it first tries to fill the slot with the highest subdivision below a specific threshold

	Let 4 be the initial threshold. Weak allocations would fill page 5 until it had 4 subdivisions, going to the next page

	We'd have to keep track of the current lowest subdivision, so that the threshold increases in the event more memory is needed

	* Macro blocks could be defined by their initial subdivision

	In the event that a weak allocation grows beyond its expected size, it will have to be moved

	For 8 macroblocks:
	4 -  1 subdivision page (64 allocations of 4kb each)
	2 -  2 subdivision page (128 allocations of 2kb each)
	2 -  4 subdivision page (256 allocations of 1kb each)

	Or each macroblock contains
	0 - 31:		1 subdivision
	32 - 47:	2 subdivision
	48 - 55:	4 subdivision
	56 - 59:	8 subdivision
	60 - 63:	16 subdivisions

	Metadata for a macroblock would be effectively:
	- 64 bits for each page, given each bit represents a 64 byte block;
	- 64 bits per macroblock to determine full allocations (possibly for each subdivision i'm willing to accept)

	Maybe it's a bad idea to define macroblocks. It creates constraints on how to split the allocations, and the only advantage is
	that one word indexes it entirely

	Don't forget the pooling idea as well! Its cost would be like one word per macroblock to assign lifetime
*/

/*
	Remember the part about allocations being aligned, you can use the bits to index
	a bigger range than possible by considering blocks instead of bytes.
	A u16 can index 4MB if the allocations are cache aligned, and 1MB if they're 16

	And we can push this even further!
	A u8 can index 16KB of memory (cache aligned), but suppose we bin it given its size
	If its upper bits can be used to index the memory region as well, then we extend it

	So for example

	Memory region for:
	
	128+ size | 64+ size | 32+ size | 16+ size | 16- size 

	This would make a cache aligned u8 index potentially 128kb!
		The catch however is that each region holds static memory, so if 128 sizes are less common
	that's memory wasted

	But maybe a better solution would be

	8- size | 9 | 10 | 11 | 12 | ... | 64

	The best solution would probably be the 4 lower bits
	The higher you go the more memory you assign to more specialized ranges
	
	The problem might be with aligned sizes
	0000 will probably be crowded

	Cache Align: 	64x
	4 Low Bit:		16x
	___________________
					1024x Expansion

	Literally 1kb expansion, u8 indexing 256kb

*/