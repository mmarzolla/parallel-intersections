/****************************************************************************
 *
 * main.cc - count intersections
 *
 * Copyright (C) 2022, 2023 Moreno Marzolla
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
#include <cassert>
#include <unistd.h>
#include <cstring>
#include "interval.hh"
#include "thrust_count.hh"
#include "utils.hh"

extern "C" {
#include <htslib/sam.h>
}

using namespace std;

void print_help(const char *exe_name)
{
    cerr << "Usage: " << exe_name << " [-N n_intervals] [-m BAM_file_name -d BED_file_name] [-n nreps]" << endl << endl
         << "where:" << endl << endl
         << "-m BAM_file_name" << endl
         << "-d BED_file_name" << endl
         << "-N n_intervals\tgenerate n_intervals random intervals (half A, half B)" << endl
         << "-r nreps\tperforms nreps replications" << endl
         << "-h\t\tThis help message" << endl << endl;
}

/**
 * Fill v with n random intervals
 */
void init( vector<interval> &v, int n )
{
    v.clear();
    for (int i=0; i<n; i++) {
        interval intrv;
        intrv.id = i;
        intrv.lower = randab(-100000, 100000);
        intrv.upper = intrv.lower + randab(10,1000);
        intrv.payload = 0;
        v.push_back(intrv);
    }
}

/**
 *
 */
void test_with_bam_and_bed( const char* bam_file_name, const char *bed_file_name, int nreps )
{
    // READ BAM
    samFile *fp_in = hts_open(bam_file_name,"r"); // open bam file
    if (fp_in == NULL) {
        cerr << "FATAL: Can not open BAM file \"" << bam_file_name << "\"" << endl;
        exit(EXIT_FAILURE);
    }
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
        i.lower = aln->core.pos + 1;
        i.upper = aln->core.pos + aln->core.l_qseq;
        i.payload = 0;
        alignments[aln->core.tid].push_back(i);
    }
    bam_destroy1(aln);
    sam_close(fp_in);
    cout << "Loaded " << alignments.size() << " alignments" << endl;

    // get target intervals from bed
    map<int32_t, vector<interval> > targets;
    ifstream bed;
    bed.open(bed_file_name);
    if (bed.fail()) {
        cerr << "FATAL: Can not open BED file \"" << bed_file_name << "\"" << endl;
        exit(EXIT_FAILURE);
    }
    string chrom_str;
    int32_t start, end;
    string line;
    while (getline(bed, line)) {
        istringstream ss(line);
        ss >> chrom_str >> start >> end;
        int32_t tid = chrom_str2tid.at(chrom_str2tid.count(chrom_str) ? chrom_str : chrom_str.substr(3));
        interval i;
        i.id = 0;
        i.lower = start;
        i.upper = end;
        i.payload = 0;
        targets[tid].push_back(i);
    }
    cout << "Loaded " << targets.size() << " target intervals" << endl;

    double intersection_time = 0;
    for (int r = 0; r<nreps; r++) {
        cout << "**" << endl
             << "** Replication " << r << " of " << nreps << endl
             << "**" << endl;
        for (auto contig = chrom_str2tid.begin(); contig != chrom_str2tid.end(); contig++) {
            const int32_t tid = contig->second;
            // cout << contig << ":" << tid << endl;
            if (alignments.count(tid) && targets.count(tid)) {
                cout << "Contig \"" <<
                    contig->first << "\" has " <<
                    alignments.at(tid).size() << " alignments and " <<
                    targets.at(tid).size() << " target intervals... ";

                // split target intervals into 1-base windows
                vector<interval> windows;
                int id = 0;
                for (auto t = targets.at(tid).begin(); t != targets.at(tid).end(); t++) {
#if 0
                    // The following loop replaces an interval [a, b] with
                    // a set of non-overlapping unitary intervals [a,
                    // a+1], [a+1, a+2], ... [b-1, b]
                    for (int32_t pos = t->lower; pos < t->upper; pos++) {
                        interval i;
                        i.id = id++;
                        i.lower = pos;
                        i.upper = pos + 1;
                        i.payload = 0;
                        windows.push_back(i);
                    }
#else
                    interval i;
                    i.id = id++;
                    i.lower = t->lower;
                    i.upper = t->upper;
                    i.payload = 0;
                    windows.push_back(i);
#endif
                }
                vector<int> counts;
                const double tstart = now();
                const int n_intersections = thrust_count(alignments.at(tid), windows, counts);
                const double elapsed = now() - tstart;
                cout << n_intersections << " intersections" << endl;
                intersection_time += elapsed;
            }
        }
    }
    cout << "**" << endl
	 << "** Average intersection time (s) " << intersection_time/nreps << endl
	 << "**" << endl << endl;
}

/**
 *
 */
void test_with_random_input(int N, int nreps)
{
    double intersection_time = 0.0;

    for (int r=0; r<nreps; r++) {
        vector<interval> A, B;
        vector<int> counts;
        cout << "**" << endl
             << "** Replication " << r << " of " << nreps << endl
             << "**" << endl;
        cout << "Generating random input..." << endl;
        init(A, N/2);
        init(B, N/2);
        const double tstart = now();
        thrust_count(A, B, counts);
        const double elapsed = now() - tstart;
        intersection_time += elapsed;
    }
    cout << "Intersection time " << intersection_time/nreps << endl;
}


int main(int argc, char *argv[])
{
    const char* bam_file_name = NULL;
    const char* bed_file_name = NULL;
    int opt;
    int N = -1;
    int nreps = 1;

    // parse command line arguments
    while ((opt = getopt(argc, argv, "hm:d:N:r:")) != -1) {
        switch (opt) {
        case 'm': // BAM file name
            bam_file_name = optarg;
            break;
        case 'd': // BED file name
            bed_file_name = optarg;
            break;
        case 'N': // generate random input
            N = atoi(optarg);
            break;
	case 'r': // number of replications
            nreps = atoi(optarg);
            break;
        default:
            cerr << "FATAL: Unrecognized option " << opt << endl << endl;
            print_help(argv[0]);
            return EXIT_FAILURE;
            break;
        }
    }

    if ((N < 0) && (bam_file_name == NULL || bed_file_name == NULL)) {
        cerr << "FATAL: You must either provide a number of intervals N"
             << "       or specify BAM and BED files using -m and -d"
             << endl
             << endl;
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    if (N > 0) {
      test_with_random_input(N, nreps);
    } else {
      test_with_bam_and_bed(bam_file_name, bed_file_name, nreps);
    }
    return EXIT_SUCCESS;
}
