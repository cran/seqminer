#include "VCFInputFile.h"
#include "Utils.h"
#include "IO.h"

#include "TabixReader.h"
#include "BCFReader.h"

// use current subset of included people
// to reconstruct a new VCF header line
void VCFInputFile::rewriteVCFHeader() {
  std::string s = "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT";
  VCFPeople& people = this->record.getPeople();
  for (unsigned int i = 0; i <people.size(); i++ ){
    s += '\t';
    s += people[i]->getName();
  }
  this->header[this->header.size()-1] = s;
}

void VCFInputFile::setRangeMode() {
  if (mode == VCF_LINE_MODE) {
    this->tabixReader = new TabixReader(this->fileName);
    if (!this->tabixReader->good()) {
      REprintf( "[ERROR] Cannot read VCF by range, please check your have index (or create one use tabix).\nQuitting...");
      //abort();
      return;
    } else {
      this->mode = VCFInputFile::VCF_RANGE_MODE;
    }
  } else if (mode == VCF_RANGE_MODE) {
    if (this->autoMergeRange) {
      this->tabixReader->mergeRange();
    }
  } else if (mode == BCF_MODE) {
    if (!this->bcfReader->good() || !this->bcfReader->indexed()) {
      REprintf( "[ERROR] Cannot read BCF by range, please check your have index (or create one use bcftools).\nQuitting...");
      // abort();
      return;
    }
    if (this->autoMergeRange) {
      this->bcfReader->mergeRange();
    }
  }

  // if (this->autoMergeRange) {
  //   this->range.sort();
  // }

  // this->rangeBegin = this->range.begin();
  // this->rangeEnd = this->range.end();
  // this->rangeIterator = this->range.begin();

}

// void VCFInputFile::clearRange() {
// #ifndef NDEBUG
//   if (this->range.size()) {
//     REprintf( "Clear existing %zu range.\n", this->range.size());
//   }
// #endif
//   if (mode == BCF_MODE) {
//     this->bcfReader->clearRange();
//   } else if (mode == VCF_RANGE_MODE) {
//     this->VCFRecord->clearRange();
//   }
//   // this->range.clear();
//   // this->ti_line = 0;
// };

/**
 * @param fn: the file contains two column: old_id new_id
 */
int VCFInputFile::updateId(const char* fn){
  // load conversion table
  LineReader lr(fn);
  std::map<std::string, std::string> tbl;
  std::vector<std::string> fd;
  while(lr.readLineBySep(&fd, "\t ")){
    if (tbl.find(fd[0]) != tbl.end()) {
      REprintf( "Duplicated original ids: [ %s ], replace it to new id anyway.\n", fd[0].c_str());
    };
    if (fd.empty() || fd[0].empty() || fd.size() < 2) continue;
    tbl[fd[0]] = fd[1];
  }

  // rewrite each people's name
  std::string s = "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT";
  VCFPeople& people = this->record.getPeople();
  int n = 0;
  for (unsigned int i = 0; i <people.size(); i++ ){
    if (tbl.find(people[i]->getName()) != tbl.end()) {
      ++n;
      people[i]->setName(tbl[people[i]->getName()]);
    }
  }
  this->rewriteVCFHeader();

  // return result
  return n;
}

void VCFInputFile::init(const char* fn) {
  this->fileName = fn;
  this->fp = NULL;
  this->tabixReader = NULL;
  this->bcfReader = NULL;

  // check whether file exists.
  FILE* fp = fopen(fn, "rb");
  if (!fp) {
    REprintf( "Cannot open file [ %s ]\n", fn);
    return;
  }
  fclose(fp);

  bool headerLoaded;
  // use file name to check file type
  if (endsWith(fn, ".bcf") || endsWith(fn, ".bcf.gz")) {
    this->mode = BCF_MODE;
    this->bcfReader = new BCFReader(fn);
    const std::string& h = this->bcfReader->getHeader();
    this->header.setHeader(h);
    this->record.createIndividual(this->header[this->header.size()-1]);
    headerLoaded = true;
  } else {
    if (!endsWith(fn, ".vcf") && !endsWith(fn, "vcf.gz")) {
      REprintf("[WARN] File name does not look like a VCF/BCF file.\n");
    }
    this->mode = VCF_LINE_MODE;
    this->fp = new LineReader(fn);

    // open file
    // read header
    while (this->fp->readLine(&line)){
      if (line[0] == '#') {
        this->header.push_back(line);
        if (line.substr(0, 6) == "#CHROM") {
          this->record.createIndividual(line);
          headerLoaded = true;
          break;
        }
        continue;
      }
      if (line[0] != '#') {
        FATAL("Wrong VCF header");
      }
    }
    //this->hasIndex = this->openIndex();
  }
  if (headerLoaded == false) {
    FATAL("VCF/BCF File does not have header!");
  }
  // this->clearRange();
}

bool VCFInputFile::readRecord(){
  // REprintf( "test\n");
  int nRead = 0;
  while (true) {
    if (this->mode == VCF_LINE_MODE) {
      nRead = this->fp->readLine(&this->line);
    } else if (this->mode == VCF_RANGE_MODE) {
      nRead = this->tabixReader->readLine(&this->line);
    } else if (this->mode == BCF_MODE) {
      nRead = this->bcfReader->readLine(&this->line);
    }
    if (!nRead) return false;

    bool ret = this->record.parse(this->line);
    if (ret) {
      reportReadError(this->line);
    }
    if (!this->passFilter())
      continue;

    // break;
    return true;
  }
}

void VCFInputFile::close() {
  // closeIndex();
  this->record.deleteIndividual();
  if (this->fp) {
    delete this->fp;
    this->fp = NULL;
  }
  if (this->tabixReader) {
    delete this->tabixReader;
    this->tabixReader = NULL;
  }
  if (this->bcfReader) {
    delete this->bcfReader;
    this->bcfReader = NULL;
  }
}

void VCFInputFile::setRangeList(const RangeList& rl){
  if (rl.size() == 0) return;

  // this->clearRange();
  // this->range = rl;
  this->setRangeMode();
  // this->clearRange();
  if (mode == VCF_RANGE_MODE) {
    this->tabixReader->setRange(rl);
  } else if (mode == BCF_MODE) {
    this->bcfReader->setRange(rl);
  } else {
    REprintf( "[ERROR] invalid reading mode, quitting...\n");
    //abort();
    return;
  }
}
