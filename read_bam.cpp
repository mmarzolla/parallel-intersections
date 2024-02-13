/****************************************************************************
 *
 * read_bam.cpp - test program to read a BAM file
 *
 * Copyright (C) 2022, 2023, 2024
 * Moreno Marzolla, Giovanni Birolo, Gabriele D'Angelo, Piero Fariselli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
extern "C" {
#include <htslib/sam.h>
}
#include "interval.hh"

using namespace std;

void intersection(const vector<interval> &a, const vector<interval> &b)
{
    // count how many intervals in a overlap each interval in b
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "usage:" << argv[0] << " BAM BED" << endl;
        return EXIT_FAILURE;
    }

    // READ BAM
    samFile *fp_in = hts_open(argv[1],"r");     // open bam file
    bam_hdr_t *bamHdr = sam_hdr_read(fp_in);    // read header
    bam1_t *aln = bam_init1();                  // initialize an alignment

    // get contig names from bam header
    map<string, int32_t> chrom_str2tid;
    for (int32_t tid = 0; tid < bamHdr->n_targets; tid++) {
        chrom_str2tid[bamHdr->target_name[tid]] = tid;
    }

    // get alignment intervals from bam
    map<int32_t, vector<interval> > alignments;
    while (sam_read1(fp_in,bamHdr,aln) > 0) {
        interval i;
        i.id = 0;
        i.left = aln->core.pos + 1;
        i.right = aln->core.pos + aln->core.l_qseq;
        i.payload = 0;
        alignments[aln->core.tid].push_back(i);
    }
    bam_destroy1(aln);
    sam_close(fp_in);
    //cout << "Loaded " << alignments.size() << " alignments" << endl;

    // get target intervals from bed
    map<int32_t, vector<interval> > targets;
    ifstream bed;
    bed.open(argv[2]);
    string chrom_str;
    int32_t start, end;
    string line;
    while (getline(bed, line)) {
        istringstream ss(line);
        ss >> chrom_str >> start >> end;
        int32_t tid = chrom_str2tid.at(chrom_str2tid.count(chrom_str) ? chrom_str : chrom_str.substr(3));
        interval i;
        i.id = 0;
        i.left = start;
        i.right = end;
        i.payload = 0;
        targets[tid].push_back(i);
    }
    //cout << "Loaded " << targets.size() << " target intervals" << endl;

    for (auto contig = chrom_str2tid.begin(); contig != chrom_str2tid.end(); contig++) {
        const int32_t tid = contig->second;
        //cout << contig << ":" << tid << endl;
        if (alignments.count(tid) && targets.count(tid)) {
            cout << "Contig \"" << contig->first << "\" has " << alignments.at(tid).size() << " alignments and " << targets.at(tid).size() << " target intervals" << endl;

            // split target intervals into 1-base windows
            vector<interval> windows;
            for (auto t = targets.at(tid).begin(); t != targets.at(tid).end(); t++)
                for (int32_t pos = t->lower; pos < t->upper; pos++) {
                    interval i;
                    i.id = 0;
                    i.left = pos;
                    i.right = pos + 1;
                    i.payload = 0;
                    windows.push_back(i);
                }
            intersection(alignments.at(tid), windows);
        }
    }

    return EXIT_SUCCESS;
}
