\name{tabix}
\alias{tabix}
\title{Read tabix file}
\usage{
  tabix(tabixFile, tabixRange)
}
\arguments{
  \item{tabixFile}{score test output file (any gzipped,
  tabix indexed file)}

  \item{tabixRange}{a text indicating which range in the
  VCF file to extract. e.g. 1:100-200}
}
\value{
  score test statistics in given range
}
\description{
  Read tabix file
}

