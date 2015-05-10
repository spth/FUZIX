/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

static readb();


/* ========	character handling for command lines	========*/


word()
{
	REG CHAR	c, d;
	REG CHAR	*argp=locstak()+BYTESPERWORD;
	INT		alpha=1;

	wdnum=0; wdset=0;

	while((c=nextc(0), space(c)) );
	if(!eofmeta(c)
	) {	do {	if(c==LITERAL
			) {	*argp++=(DQUOTE);
				while((c=readc()) && c!=LITERAL
				DO *argp++=(c|QUOTE); chkpr(c) OD
				*argp++=(DQUOTE);
			} else {	*argp++=(c);
				if(c=='=' ) { wdset |= alpha ;}
				if(!alphanum(c) ) { alpha=0 ;}
				if(qotchar(c)
				) {	d=c;
					while((*argp++=(c=nextc(d))) && c!=d
					DO chkpr(c) OD
				;}
			;}
		} while ( (c=nextc(0), !eofmeta(c)) );
		argp=endstak(argp);
		if(!letter(((ARGPTR)argp)->argval[0]) ) { wdset=0 ;}

		peekc=c|MARK;
		if(((ARGPTR)argp)->argval[1]==0 && (d=((ARGPTR)argp)->argval[0], digit(d)) && (c=='>' || c=='<')
		) {	word(); wdnum=d-'0';
		} else {	/*check for reserved words*/
			if(reserv==FALSE || (wdval=syslook(((ARGPTR)argp)->argval,reserved))==0
			) {	wdarg=(ARGPTR)argp; wdval=0;
			;}
		;}

	} else if ( dipchar(c)
	) {	if((d=nextc(0))==c
		) {	wdval = c|SYMREP;
		} else {	peekc = d|MARK; wdval = c;
		;}
	} else {	if((wdval=c)==EOF
		) {	wdval=EOFSYM;
		;}
		if(iopend && eolchar(c)
		) {	copy(iopend); iopend=0;
		;}
	;}
	reserv=FALSE;
	return(wdval);
}

nextc(quote)
	CHAR		quote;
{
	REG CHAR	c, d;
	if((d=readc())==ESCAPE
	) {	if((c=readc())==NL
		) {	chkpr(NL); d=nextc(quote);
		} else if ( quote && c!=quote && !escchar(c)
		) {	peekc=c|MARK;
		} else {	d = c|QUOTE;
		;}
	;}
	return(d);
}

readc()
{
	REG CHAR	c;
	REG INT		len;
	REG FILE	f;

retry:
	if(peekc
	) {	c=peekc; peekc=0;
	} else if ( (f=standin, f->fnxt!=f->fend)
	) {	if((c = *f->fnxt++)==0
		) {	if(f->feval
			) {	if(estabf(*f->feval++)
				) {	c=EOF;
				} else {	c=SP;
				;}
			} else {	goto retry; /* = c=readc(); */
			;}
		;}
		if(flags&readpr && standin->fstak==0 ) { prc(c) ;}
		if(c==NL ) { f->flin++ ;}
	} else if ( f->feof || f->fdes<0
	) {	c=EOF; f->feof++;
	} else if ( (len=readb())<=0
	) {	close(f->fdes); f->fdes = -1; c=EOF; f->feof++;
	} else {	f->fend = (f->fnxt = f->fbuf)+len;
		goto retry;
	;}
	return(c);
}

static readb()
{
	REG FILE	f=standin;
	REG INT		len;

	do {	if(trapnote&SIGSET ) { newline(); sigchk() ;}
	} while ( (len=read(f->fdes,f->fbuf,f->fsiz))<0 && trapnote );
	return(len);
}
