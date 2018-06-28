#include <bitset>
#include <string>
#include "algorithms/SPRING/BooPHF.h"

void bbhashdict::findpos(int64_t *dictidx, uint32_t &startposidx) {
  dictidx[0] = startpos[startposidx];
  auto endidx = startpos[startposidx + 1];
  if (read_id[endidx - 1] ==
      numreads)  // means exactly one read has been removed
    dictidx[1] = endidx - 1;
  else if (read_id[endidx - 1] == numreads + 1)  // means two or more reads have
                                                 // been removed (in this case
                                                 // second last entry stores the
                                                 // number of reads left)
    dictidx[1] = dictidx[0] + read_id[endidx - 2];
  else
    dictidx[1] = endidx;  // no read deleted
  return;
}

void bbhashdict::remove(int64_t *dictidx, uint32_t &startposidx,
                        uint32_t current) {
  auto size = dictidx[1] - dictidx[0];
  if (size == 1)  // just one read left in bin
  {
    empty_bin[startposidx] = 1;
    return;  // need to keep one read to check during matching
  }
  uint32_t pos =
      std::lower_bound(read_id + dictidx[0], read_id + dictidx[1], current) -
      (read_id + dictidx[0]);

  for (uint32_t i = dictidx[0] + pos; i < dictidx[1] - 1; i++)
    read_id[i] = read_id[i + 1];
  auto endidx = startpos[startposidx + 1];
  if (dictidx[1] == endidx)  // this is first read to be deleted
    read_id[endidx - 1] = numreads;
  else if (read_id[endidx - 1] ==
           numreads)  // exactly one read has been deleted till now
  {
    read_id[endidx - 1] = numreads + 1;
    read_id[endidx - 2] = size - 1;  // number of reads left in bin
  } else                             // more than two reads have been deleted
    read_id[endidx - 2]--;

  return;
}

template<size_t bitset_size>
void stringtobitset(std::string s, uint8_t readlen, std::bitset<bitset_size> &b, std::bitset<bitset_size> basemask[256][128]) {
  for (int i = 0; i < readlen; i++) b |= basemask[i][s[i]];
}

template<size_t bitset_size>
void constructdictionary(std::bitset<bitset_size> *read, bbhashdict *dict, uint8_t *read_lengths, int numdict, uint32_t numreads, int bpb) {
  std::bitset<bitset_size> mask[numdict];
  generateindexmasks(mask);
  for (int j = 0; j < numdict; j++) {
    uint64_t *ull = new uint64_t[numreads];
#pragma omp parallel
    {
      std::bitset<bitset_size> b;
      int tid = omp_get_thread_num();
      uint32_t i, stop;
      i = uint64_t(tid) * numreads / omp_get_num_threads();
      stop = uint64_t(tid + 1) * numreads / omp_get_num_threads();
      if (tid == omp_get_num_threads() - 1) stop = numreads;
      // compute keys and and store in ull
      for (; i < stop; i++) {
        b = read[i] & mask[j];
        ull[i] = (b >> bpb * dict[j].start).to_ullong();
      }
    }  // parallel end

    // remove keys corresponding to reads shorter than dict_end[j]
    dict[j].dict_numreads = 0;
    for (uint32_t i = 0; i < numreads; i++) {
      if (read_lengths[i] > dict_end[j]) {
        ull[dict[j].dict_numreads] = ull[i];
        dict[j].dict_numreads++;
      }
    }

// write ull to file
#pragma omp parallel
    {
      int tid = omp_get_thread_num();
      std::ofstream foutkey(
          basedir + std::string("/keys.bin.") + std::to_string(tid),
          std::ios::binary);
      uint32_t i, stop;
      i = uint64_t(tid) * dict[j].dict_numreads / omp_get_num_threads();
      stop = uint64_t(tid + 1) * dict[j].dict_numreads / omp_get_num_threads();
      if (tid == omp_get_num_threads() - 1) stop = dict[j].dict_numreads;
      for (; i < stop; i++) foutkey.write((char *)&ull[i], sizeof(uint64_t));
      foutkey.close();
    }  // parallel end

    // deduplicating ull
    std::sort(ull, ull + dict[j].dict_numreads);
    uint32_t k = 0;
    for (uint32_t i = 1; i < dict[j].dict_numreads; i++)
      if (ull[i] != ull[k]) ull[++k] = ull[i];
    dict[j].numkeys = k + 1;
    // construct mphf
    auto data_iterator =
        boomphf::range(static_cast<const u_int64_t *>(ull),
                       static_cast<const u_int64_t *>(ull + dict[j].numkeys));
    double gammaFactor = 5.0;  // balance between speed and memory
    dict[j].bphf = new boomphf::mphf<u_int64_t, hasher_t>(
        dict[j].numkeys, data_iterator, num_thr, gammaFactor, true, false);

    delete[] ull;

// compute hashes for all reads
#pragma omp parallel
    {
      int tid = omp_get_thread_num();
      std::ifstream finkey(
          basedir + std::string("/keys.bin.") + std::to_string(tid),
          std::ios::binary);
      std::ofstream fouthash(basedir + std::string("/hash.bin.") +
                                 std::to_string(tid) + '.' + std::to_string(j),
                             std::ios::binary);
      uint64_t currentkey, currenthash;
      uint32_t i, stop;
      i = uint64_t(tid) * dict[j].dict_numreads / omp_get_num_threads();
      stop = uint64_t(tid + 1) * dict[j].dict_numreads / omp_get_num_threads();
      if (tid == omp_get_num_threads() - 1) stop = dict[j].dict_numreads;
      for (; i < stop; i++) {
        finkey.read((char *)&currentkey, sizeof(uint64_t));
        currenthash = (dict[j].bphf)->lookup(currentkey);
        fouthash.write((char *)&currenthash, sizeof(uint64_t));
      }
      finkey.close();
      remove(
          (basedir + std::string("/keys.bin.") + std::to_string(tid)).c_str());
      fouthash.close();
    }  // parallel end
  }

  // for rest of the function, use numdict threads to parallelize
  omp_set_num_threads(std::min(numdict, num_thr));
#pragma omp parallel
  {
#pragma omp for
    for (int j = 0; j < numdict; j++) {
      // fill startpos by first storing numbers and then doing cumulative sum
      dict[j].startpos =
          new uint32_t[dict[j].numkeys +
                       1]();  // 1 extra to store end pos of last key
      uint64_t currenthash;
      for (int tid = 0; tid < num_thr; tid++) {
        std::ifstream finhash(basedir + std::string("/hash.bin.") +
                                  std::to_string(tid) + '.' + std::to_string(j),
                              std::ios::binary);
        finhash.read((char *)&currenthash, sizeof(uint64_t));
        while (!finhash.eof()) {
          dict[j].startpos[currenthash + 1]++;
          finhash.read((char *)&currenthash, sizeof(uint64_t));
        }
        finhash.close();
      }

      dict[j].empty_bin = new bool[dict[j].numkeys]();
      for (uint32_t i = 1; i < dict[j].numkeys; i++)
        dict[j].startpos[i] = dict[j].startpos[i] + dict[j].startpos[i - 1];

      // insert elements in the dict array
      dict[j].read_id = new uint32_t[dict[j].dict_numreads];
      uint32_t i = 0;
      for (int tid = 0; tid < num_thr; tid++) {
        std::ifstream finhash(basedir + std::string("/hash.bin.") +
                                  std::to_string(tid) + '.' + std::to_string(j),
                              std::ios::binary);
        finhash.read((char *)&currenthash, sizeof(uint64_t));
        while (!finhash.eof()) {
          while (read_lengths[i] <= dict_end[j]) i++;
          dict[j].read_id[dict[j].startpos[currenthash]++] = i;
          i++;
          finhash.read((char *)&currenthash, sizeof(uint64_t));
        }
        finhash.close();
        remove((basedir + std::string("/hash.bin.") + std::to_string(tid) +
                '.' + std::to_string(j))
                   .c_str());
      }

      // correcting startpos array modified during insertion
      for (int64_t i = dict[j].numkeys; i >= 1; i--)
        dict[j].startpos[i] = dict[j].startpos[i - 1];
      dict[j].startpos[0] = 0;
    }  // for end
  }    // parallel end
  omp_set_num_threads(num_thr);
  return;
}

template<size_t MAX_READ_LEN, size_t bitset_size>
void generatemasks(std::bitset<bitset_size> mask[MAX_READ_LEN][MAX_READ_LEN], uint8_t max_readlen, int bpb) {
// mask for zeroing the end bits (needed while reordering to compute Hamming
// distance between shifted reads)
  for (int i = 0; i < max_readlen; i++) {
    for (int j = 0; j < max_readlen; j++) {
      mask[i][j].reset();
      for (int k = bpb * i; k < bpb * max_readlen - bpb * j; k++) mask[i][j][k] = 1;
    }
  }
  return;
}

void reverse_complement(char *s, char *s1, uint8_t readlen, char chartorevchar[128]) {
  for (int j = 0; j < readlen; j++) s1[j] = chartorevchar[s[readlen - j - 1]];
  s1[readlen] = '\0';
  return;
}

template<size_t bitset_size>
void chartobitset(char *s, uint8_t readlen, std::bitset<bitset_size> &b, std::bitset<bitset_size> basemask[256][128]) {
  b.reset();
  for (int i = 0; i < readlen; i++) b |= basemask[i][s[i]];
  return;
}

std::string reverse_complement(std::string s, uint8_t readlen, char chartorevchar[128]) {
  std::string s1(s);
  for (int j = 0; j < readlen; j++) s1[j] = chartorevchar[s[readlen - j - 1]];
  return s1;
}

template<size_t bitset_size>
void generateindexmasks(std::bitset<bitset_size> *mask1, bbhashdict *dict, int numdict, int bpb) {
  for (int j = 0; j < numdict; j++) mask1[j].reset();
  for (int j = 0; j < numdict; j++)
    for (int i = bpb * dict[j].start; i < bpb * (dict[j].end + 1); i++)
      mask1[j][i] = 1;
  return;
}
