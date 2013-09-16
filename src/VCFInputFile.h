#ifndef _VCFINPUTFILE_H_
#define _VCFINPUTFILE_H_

// #include "tabix.h"
#include "VCFRecord.h"
#include "VCFFilter.h"

class TabixReader;
class BCFReader;

/**
 * parse is equivalent to copy, meaning we will copy the content to speed up later reference using const char*
 * three modes to read a VCF files
 *  (1) BCF file mode, read by region or by line
 *  (2) VCF file, read by line
 *  (3) VCF file, read by region
 */
class VCFInputFile{
 public:
  typedef enum {
    BCF_MODE,
    VCF_LINE_MODE,          // read by line
    VCF_RANGE_MODE          // read by range
  } Mode;

 private:
  // disable copy-constructor
  VCFInputFile(const VCFInputFile&);
  VCFInputFile& operator=(const VCFInputFile&);

 public:
  VCFInputFile (const std::string& fn) {
    init(fn.c_str());
  }
  
  VCFInputFile (const char* fn) {
    init(fn);
  }
  void init(const char* fn);
  
  virtual ~VCFInputFile(){
    this->close();
  }
  void close();
  
  VCFHeader* getVCFHeader() {
    this->rewriteVCFHeader();
    return &this->header;
  };

  // use current subset of included people
  // to reconstruct a new VCF header line
  void rewriteVCFHeader();
  //  void setRangeMode();

  /**
   * Report a line that does not conform to VCF standard.
   */
  void reportReadError(const std::string& line) {
    if (line.size() > 50) {
      REprintf( "Error line [ %s ... ]\n", line.substr(0, 50).c_str());
    } else {
      REprintf( "Error line [ %s ]\n", line.c_str());
    }
  }
  /**
   * Check with VCFFilter to see if the current read line passed
   */
  virtual bool passFilter() {
    return true;
  };
  /**
   * @return true: a valid VCFRecord
   */
  bool readRecord();

  //////////////////////////////////////////////////
  // Sample inclusion/exclusion
  void includePeople(const char* s) {
    this->record.includePeople(s);
  }
  void includePeople(const std::vector<std::string>& v){
    this->record.includePeople(v);
  }
  void includePeopleFromFile(const char* fn) {
    this->record.includePeopleFromFile(fn);
  }
  void includeAllPeople(){
    this->record.includeAllPeople();
  }
  void excludePeople(const char* s) {
    this->record.excludePeople(s);
  }
  void excludePeople(const std::vector<std::string>& v){
    this->record.excludePeople(v);
  }
  void excludePeopleFromFile(const char* fn) {
    this->record.excludePeopleFromFile(fn);
  }
  void excludeAllPeople(){
    this->record.excludeAllPeople();
  }
  //////////////////////////////////////////////////
  // Adjust range collections
  void enableAutoMerge() {
    this->autoMergeRange = true;
  }
  void disableAutoMerge() {
    this->autoMergeRange = false;
  }
  // void clearRange();
  void setRangeFile(const char* fn) {
    if (!fn || strlen(fn) == 0)
      return;
    RangeList r;
    r.addRangeFile(fn);
    this->setRange(r);
  };
  // @param l is a string of range(s)
  void setRange(const char* chrom, int begin, int end) {
    RangeList r;
    r.addRange(chrom, begin, end);
    this->setRange(r);
  }
  void setRange(const RangeList& rl) {
    this->setRangeList(rl);
  }
  void setRangeList(const std::string& l){
    if (l.empty())
      return;

    RangeList r;
    r.addRangeList(l);
    this->setRange(r);
  }
  void setRangeList(const RangeList& rl);

  /**
   * @param fn load file and use first column as old id, second column as new id
   * @return number of ids have been changed.
   */
  int updateId(const char* fn);

  VCFRecord& getVCFRecord() {return this->record;};
  const char* getLine() const {return this->line.c_str();};
  const char* getFileName() const {return this->fileName.c_str();};

 private:
  void setRangeMode();
  
 private:
  VCFHeader header;
  VCFRecord record;

  // tabix_t * tabixHandle;
  // bool hasIndex;
  // RangeList range;

  std::string fileName;

  // variable used for accessing by rrange
  RangeList::iterator rangeBegin;
  RangeList::iterator rangeEnd;
  RangeList::iterator rangeIterator;
  // ti_iter_t iter;
  // const char* ti_line;
  bool autoMergeRange;

  Mode mode;
  std::string line;
  bool isBCF; // read a bcf file?

  // readers
  LineReader* fp;
  TabixReader* tabixReader;
  BCFReader* bcfReader;
};

#endif /* _VCFINPUTFILE_H_ */
