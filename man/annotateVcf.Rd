% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/seqminer.R
\name{annotateVcf}
\alias{annotateVcf}
\title{Annotate a VCF file}
\usage{
annotateVcf(inVcf, outVcf, params)
}
\arguments{
\item{inVcf}{input VCF file name}

\item{outVcf}{output VCF file name}

\item{params}{parameters}
}
\value{
0 if succeed
}
\description{
Annotate a VCF file
}
\examples{
param <- list(reference = system.file("tabanno/test.fa", package = "seqminer"),
              geneFile = system.file("tabanno/test.gene.txt", package = "seqminer"))
param <- makeAnnotationParameter(param)
inVcf <- system.file("tabanno/input.test.vcf", package = "seqminer")
outVcf <- file.path(tempdir(), "/", "out.vcf")
annotateVcf (inVcf, outVcf, param)
cat('Annotated VCF files are in the temp directory:', outVcf, '\n')
}
