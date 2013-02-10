/*************** TabFmt H Declares Source Code File (.H) ***************/
/*  Name: TABFMT.H    Version 2.3                                      */
/*                                                                     */
/*  (C) Copyright to the author Olivier BERTRAND          2001-2013    */
/*                                                                     */
/*  This file contains the CSV and FMT classes declares.               */
/***********************************************************************/
#include "xtable.h"                     // Base class declares
#include "tabdos.h"

//pedef struct _tabdesc  *PTABD;        // For friend setting
typedef class  TDBFMT    *PTDBFMT;

/***********************************************************************/
/*  Functions used externally.                                         */
/***********************************************************************/
PQRYRES CSVColumns(PGLOBAL g, char *fn, char sep, char q,
                   int hdr, int mxr, bool info);

/***********************************************************************/
/*  CSV table.                                                         */
/***********************************************************************/
class DllExport CSVDEF : public DOSDEF { /* Logical table description  */
  friend class TDBCSV;
  friend class TDBCCL;
 public:
  // Constructor
  CSVDEF(void);

  // Implementation
  virtual const char *GetType(void) {return "CSV";}
  char    GetSep(void) {return Sep;}
  char    GetQot(void) {return Qot;}

  // Methods
  virtual bool DefineAM(PGLOBAL g, LPCSTR am, int poff);
  virtual PTDB GetTable(PGLOBAL g, MODE mode);

 protected:
  // Members
  bool    Fmtd;               /* true for formatted files              */
//bool    Accept;             /* true if wrong lines are accepted      */
  bool    Header;             /* true if first line contains headers   */
//int     Maxerr;             /* Maximum number of bad records         */
  int     Quoted;             /* Quoting level for quoted fields       */
  char    Sep;                /* Separator for standard CSV files      */
  char    Qot;                /* Character for quoted strings          */
  }; // end of CSVDEF

/***********************************************************************/
/*  This is the DOS/UNIX Access Method class declaration for files     */
/*  that are CSV files with columns separated by the Sep character.    */
/***********************************************************************/
class TDBCSV : public TDBDOS {
  friend class CSVCOL;
 public:
  // Constructor
  TDBCSV(PCSVDEF tdp, PTXF txfp);
  TDBCSV(PGLOBAL g, PTDBCSV tdbp);

  // Implementation
  virtual AMT  GetAmType(void) {return TYPE_AM_CSV;}
  virtual PTDB Duplicate(PGLOBAL g)
                {return (PTDB)new(g) TDBCSV(g, this);}

  // Methods
  virtual PTDB CopyOne(PTABS t);
//virtual bool IsUsingTemp(PGLOBAL g);
  virtual int  GetBadLines(void) {return (int)Nerr;}

  // Database routines
  virtual PCOL MakeCol(PGLOBAL g, PCOLDEF cdp, PCOL cprec, int n);
  virtual bool OpenDB(PGLOBAL g);
  virtual int  WriteDB(PGLOBAL g);
  virtual int  CheckWrite(PGLOBAL g);
  virtual int  ReadBuffer(PGLOBAL g);        // Physical file read

  // Specific routines
  virtual int  EstimatedLength(PGLOBAL g);
  virtual bool SkipHeader(PGLOBAL g);
  virtual bool CheckErr(void);

 protected:
  // Members
  PSZ  *Field;             // Field to write to current line
  int  *Offset;            // Column offsets for current record
  int  *Fldlen;            // Column field length for current record
  bool *Fldtyp;            // true for numeric fields
  int   Fields;            // Number of fields to handle
  int   Nerr;              // Number of bad records
  int   Maxerr;            // Maximum number of bad records
  int   Quoted;            // Quoting level for quoted fields
  bool  Accept;             // true if bad lines are accepted
  bool  Header;             // true if first line contains column headers
  char  Sep;               // Separator
  char  Qot;               // Quoting character
  }; // end of class TDBCSV

/***********************************************************************/
/*  Class CSVCOL: CSV access method column descriptor.                 */
/*  This A.M. is used for Comma Separated V(?) files.                  */
/***********************************************************************/
class CSVCOL : public DOSCOL {
  friend class TDBCSV;
  friend class TDBFMT;
 public:
  // Constructors
  CSVCOL(PGLOBAL g, PCOLDEF cdp, PTDB tdbp, PCOL cprec, int i);
  CSVCOL(CSVCOL *colp, PTDB tdbp); // Constructor used in copy process

  // Implementation
  virtual int    GetAmType() {return TYPE_AM_CSV;}

  // Methods
  virtual bool   VarSize(void);
  virtual void   ReadColumn(PGLOBAL g);
  virtual void   WriteColumn(PGLOBAL g);
//        void   Print(FILE *, uint);

 protected:
  // Default constructor not to be used
  CSVCOL(void) {}

  // Members
  int Fldnum;               // Field ordinal number (0 based)
  }; // end of class CSVCOL

/***********************************************************************/
/*  This is the DOS/UNIX Access Method class declaration for files     */
/*  whose record format is described by a Format keyword.              */
/***********************************************************************/
class TDBFMT : public TDBCSV {
  friend class CSVCOL;
//friend class FMTCOL;
 public:
  // Standard constructor
  TDBFMT(PCSVDEF tdp, PTXF txfp) : TDBCSV(tdp, txfp)
      {FldFormat = NULL; To_Fld = NULL; FmtTest = NULL; Linenum = 0;}

  // Copy constructor
  TDBFMT(PGLOBAL g, PTDBFMT tdbp);

  // Implementation
  virtual AMT  GetAmType(void) {return TYPE_AM_FMT;}
  virtual PTDB Duplicate(PGLOBAL g)
                {return (PTDB)new(g) TDBFMT(g, this);}

  // Methods
  virtual PTDB CopyOne(PTABS t);

  // Database routines
  virtual PCOL MakeCol(PGLOBAL g, PCOLDEF cdp, PCOL cprec, int n);
//virtual int  GetMaxSize(PGLOBAL g);
  virtual bool OpenDB(PGLOBAL g);
  virtual int  WriteDB(PGLOBAL g);
//virtual int  CheckWrite(PGLOBAL g);
  virtual int  ReadBuffer(PGLOBAL g);        // Physical file read

  // Specific routines
  virtual int  EstimatedLength(PGLOBAL g);

 protected:
  // Members
  PSZ  *FldFormat;                      // Field read format
  void *To_Fld;                         // To field test buffer
  int  *FmtTest;                        // Test on ending by %n or %m
  int   Linenum;                        // Last read line
  }; // end of class TDBFMT

/***********************************************************************/
/*  This is the class declaration for the CSV catalog table.           */
/***********************************************************************/
class TDBCCL : public TDBCAT {
 public:
  // Constructor
  TDBCCL(PCSVDEF tdp);

 protected:
	// Specific routines
	virtual PQRYRES GetResult(PGLOBAL g);

  // Members
  char   *Fn;                     // The CSV file (path) name
  bool    Hdr;                    // true if first line contains headers
  int     Mxr;                    // Maximum number of bad records
  int     Qtd;                    // Quoting level for quoted fields
  char    Sep;                    // Separator for standard CSV files 
  }; // end of class TDBCCL

/* ------------------------- End of TabFmt.H ------------------------- */
