/******************************************************************************
 *
 * 
 *
 *
 * Copyright (C) 1997-2002 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#ifndef _HTMLDOCVISITOR_H
#define _HTMLDOCVISITOR_H

#include "docvisitor.h"
#include "docparser.h"
#include "language.h"
#include "doxygen.h"
#include "outputgen.h"
#include "code.h"
#include "dot.h"

/*! @brief Concrete visitor implementation for HTML output. */
class HtmlDocVisitor : public DocVisitor
{
  public:
    HtmlDocVisitor(QTextStream &t,BaseCodeDocInterface &ci) 
      : m_t(t), m_ci(ci), m_insidePre(FALSE), m_hide(FALSE) {}
    
    //--------------------------------------
    // visitor functions for leaf nodes
    //--------------------------------------
    
    void visit(DocWord *w)
    {
      if (m_hide) return;
      filter(w->word());
    }
    void visit(DocLinkedWord *w)
    {
      if (m_hide) return;
      startLink(w->ref(),w->file(),w->anchor());
      filter(w->word());
      endLink();
    }
    void visit(DocWhiteSpace *w)
    {
      if (m_hide) return;
      if (m_insidePre)
      {
        m_t << w->chars();
      }
      else
      {
        m_t << " ";
      }
    }
    void visit(DocSymbol *s)
    {
      if (m_hide) return;
      switch(s->symbol())
      {
	case DocSymbol::BSlash:  m_t << "\\"; break;
	case DocSymbol::At:      m_t << "@"; break;
	case DocSymbol::Less:    m_t << "&lt;"; break;
	case DocSymbol::Greater: m_t << "&gt;"; break;
	case DocSymbol::Amp:     m_t << "&amp;"; break;
	case DocSymbol::Dollar:  m_t << "$"; break;
	case DocSymbol::Hash:    m_t << "#"; break;
	case DocSymbol::Percent: m_t << "%"; break;
	case DocSymbol::Copy:    m_t << "&copy;"; break;
	case DocSymbol::Apos:    m_t << "'"; break;
	case DocSymbol::Quot:    m_t << "\""; break;
	case DocSymbol::Uml:     m_t << "&" << s->letter() << "uml;"; break;
	case DocSymbol::Acute:   m_t << "&" << s->letter() << "acute;"; break;
	case DocSymbol::Grave:   m_t << "&" << s->letter() << "grave;"; break;
	case DocSymbol::Circ:    m_t << "&" << s->letter() << "circ;"; break;
	case DocSymbol::Tilde:   m_t << "&" << s->letter() << "tilde;"; break;
	case DocSymbol::Szlig:   m_t << "&szlig;"; break;
	case DocSymbol::Cedil:   m_t << "&" << s->letter() << "cedul;"; break;
	case DocSymbol::Ring:    m_t << "&" << s->letter() << "ring;"; break;
	case DocSymbol::Nbsp:    m_t << "&nbsp;"; break;
	default:
	  printf("Error: unknown symbol found\n");
      }
    }
    void visit(DocURL *u)
    {
      if (m_hide) return;
      m_t << "<a href=\"" << u->url() << "\">" << u->url() << "</a>";
    }
    void visit(DocLineBreak *)
    {
      if (m_hide) return;
      m_t << "<br>\n";
    }
    void visit(DocHorRuler *)
    {
      if (m_hide) return;
      m_t << "<hr>\n";
    }
    void visit(DocStyleChange *s)
    {
      if (m_hide) return;
      switch (s->style())
      {
        case DocStyleChange::Bold:
         if (s->enable()) m_t << "<b>";      else m_t << "</b> ";
         break;
        case DocStyleChange::Italic:
         if (s->enable()) m_t << "<em>";     else m_t << "</em> ";
         break;
        case DocStyleChange::Code:
         if (s->enable()) m_t << "<code>";   else m_t << "</code> ";
         break;
        case DocStyleChange::Subscript:
         if (s->enable()) m_t << "<sub>";    else m_t << "</sub> ";
         break;
        case DocStyleChange::Superscript:
         if (s->enable()) m_t << "<sup>";    else m_t << "</sup> ";
         break;
        case DocStyleChange::Center:
         if (s->enable()) m_t << "<center>"; else m_t << "</center> ";
         break;
        case DocStyleChange::Small:
         if (s->enable()) m_t << "<small>";  else m_t << "</small> ";
         break;
      }
    }
    void visit(DocVerbatim *s)
    {
      if (m_hide) return;
      switch(s->type())
      {
        case DocVerbatim::Code: // fall though
           m_t << "<div class=\"fragment\"><pre>"; 
           parseCode(m_ci,s->context(),s->text(),FALSE,0);
           m_t << "</pre></div>"; 
           break;
        case DocVerbatim::Verbatim: 
           m_t << "<div class=\"fragment\"><pre>";
           filter(s->text());
           m_t << "</pre></div>"; 
           break;
        case DocVerbatim::HtmlOnly: 
           m_t << s->text(); 
           break;
        case DocVerbatim::LatexOnly: 
           /* nothing */ 
           break;
      }
    }
    void visit(DocAnchor *)
    {
      if (m_hide) return;
      m_t << "<a name=\"%s\"/></a>";
    }
    void visit(DocInclude *inc)
    {
      if (m_hide) return;
      switch(inc->type())
      {
        case DocInclude::Include: 
          m_t << "<div class=\"fragment\"><pre>";
          parseCode(m_ci,inc->context(),inc->text(),FALSE,0);
          m_t << "</pre></div>"; 
          break;
        case DocInclude::DontInclude: 
          break;
        case DocInclude::HtmlInclude: 
          m_t << inc->text(); 
          break;
        case DocInclude::VerbInclude: 
          m_t << "<div class=\"fragment\"><pre>";
          filter(inc->text());
          m_t << "</pre></div>"; 
          break;
      }
    }
    void visit(DocIncOperator *op)
    {
      //printf("DocIncOperator: type=%d first=%d, last=%d text=`%s'\n",
      //    op->type(),op->isFirst(),op->isLast(),op->text().data());
      if (op->isFirst()) 
      {
        m_t << "<div class=\"fragment\"><pre>";
        m_hide = TRUE;
      }
      if (op->type()!=DocIncOperator::Skip) 
      {
        parseCode(m_ci,op->context(),op->text(),FALSE,0);
      }
      if (op->isLast())  
      {
        m_hide = FALSE;
        m_t << "</pre></div>"; 
      }
      else
      {
        m_t << endl;
      }
    }
    void visit(DocFormula *f)
    {
      if (m_hide) return;
      if (f->text().at(0)=='\\') m_t << "<p><center>" << endl;
      m_t << "<img align=";
#if !defined(_WIN32)
      m_t << "\"top\"";     // assume Unix users use Netscape 4.x which does
                            // not seem to support align == "middle" :-((
#else
      m_t << "\"middle\"";  // assume Windows users use IE or HtmlHelp which on
                            // displays formulas nicely with align == "middle" 
#endif
      m_t << " src=\"" << f->name() << ".png\">";
      if (f->text().at(0)=='\\') 
        m_t << endl << "</center><p>" << endl;
      else
        m_t << " ";
    }

    //--------------------------------------
    // visitor functions for compound nodes
    //--------------------------------------
    
    void visitPre(DocAutoList *l)
    {
      if (l->isEnumList())
      {
        m_t << "<ol>\n";
      }
      else
      {
        m_t << "<ul>\n";
      }
    }
    void visitPost(DocAutoList *l)
    {
      if (l->isEnumList())
      {
        m_t << "</ol>\n";
      }
      else
      {
        m_t << "</ul>\n";
      }
    }
    void visitPre(DocAutoListItem *)
    {
      m_t << "<li>";
    }
    void visitPost(DocAutoListItem *) 
    {
      m_t << "</li>";
    }
    void visitPre(DocPara *) 
    {
    }
    void visitPost(DocPara *p)
    {
      if (!p->isLast() &&            // omit <p> for last paragraph
          !(p->parent() &&           // and for parameter sections
            p->parent()->kind()==DocNode::Kind_ParamSect
           )
         ) m_t << "\n<p>\n";
    }
    void visitPre(DocRoot *)
    {
      //m_t << "<hr><h4><font color=\"red\">New parser:</font></h4>\n";
    }
    void visitPost(DocRoot *)
    {
      //m_t << "<hr><h4><font color=\"red\">Old parser:</font></h4>\n";
    }
    void visitPre(DocSimpleSect *s)
    {
      m_t << "<dl compact><dt><b>";
      switch(s->type())
      {
	case DocSimpleSect::See: 
          m_t << theTranslator->trSeeAlso(); break;
	case DocSimpleSect::Return: 
          m_t << theTranslator->trReturns(); break;
	case DocSimpleSect::Author: 
          m_t << theTranslator->trAuthor(TRUE,TRUE); break;
	case DocSimpleSect::Authors: 
          m_t << theTranslator->trAuthor(TRUE,FALSE); break;
	case DocSimpleSect::Version: 
          m_t << theTranslator->trVersion(); break;
	case DocSimpleSect::Since: 
          m_t << theTranslator->trSince(); break;
	case DocSimpleSect::Date: 
          m_t << theTranslator->trDate(); break;
	case DocSimpleSect::Note: 
          m_t << theTranslator->trNote(); break;
	case DocSimpleSect::Warning:
          m_t << theTranslator->trWarning(); break;
	case DocSimpleSect::Pre:
          m_t << theTranslator->trPrecondition(); break;
	case DocSimpleSect::Post:
          m_t << theTranslator->trPostcondition(); break;
	case DocSimpleSect::Invar:
          m_t << theTranslator->trInvariant(); break;
	case DocSimpleSect::Remark:
          m_t << theTranslator->trRemarks(); break;
	case DocSimpleSect::Attention:
          m_t << theTranslator->trAttention(); break;
	case DocSimpleSect::User: break;
	case DocSimpleSect::Unknown:  break;
      }

      // special case 1: user defined title
      if (s->type()!=DocSimpleSect::User)
      {
        m_t << ":</b></dt><dd>";
      }
    }
    void visitPost(DocSimpleSect *)
    {
      m_t << "</dd></dl>\n";
    }
    void visitPre(DocTitle *)
    {
    }
    void visitPost(DocTitle *)
    {
      m_t << "</b></dt><dd>";
    }
    void visitPre(DocSimpleList *)
    {
      m_t << "<ul>\n";
    }
    void visitPost(DocSimpleList *)
    {
      m_t << "</ul>\n";
    }
    void visitPre(DocSimpleListItem *)
    {
      m_t << "<li>";
    }
    void visitPost(DocSimpleListItem *) 
    {
      m_t << "</li>\n";
    }
    void visitPre(DocSection *s)
    {
      m_t << "<h" << s->level()+1 << ">";
      m_t << "<a name=\"" << s->anchor();
      filter(s->title());
      m_t << "\"</a>" << endl;
      m_t << "</h" << s->level()+1 << ">\n";
    }
    void visitPost(DocSection *) 
    {
    }
    void visitPre(DocHtmlList *s)
    {
      if (s->type()==DocHtmlList::Ordered) 
        m_t << "<ol>\n"; 
      else 
        m_t << "<ul>\n";
    }
    void visitPost(DocHtmlList *s) 
    {
      if (s->type()==DocHtmlList::Ordered) 
        m_t << "</ol>\n"; 
      else 
        m_t << "</ul>\n";
    }
    void visitPre(DocHtmlListItem *)
    {
      m_t << "<li>\n";
    }
    void visitPost(DocHtmlListItem *) 
    {
      m_t << "</li>\n";
    }
    void visitPre(DocHtmlPre *)
    {
      m_t << "<pre>\n";
      m_insidePre=TRUE;
    }
    void visitPost(DocHtmlPre *) 
    {
      m_insidePre=FALSE;
      m_t << "</pre>\n";
    }
    void visitPre(DocHtmlDescList *)
    {
      m_t << "<dl>\n";
    }
    void visitPost(DocHtmlDescList *) 
    {
      m_t << "</dl>\n";
    }
    void visitPre(DocHtmlDescTitle *)
    {
      m_t << "<dt>";
    }
    void visitPost(DocHtmlDescTitle *) 
    {
      m_t << "</dt>\n";
    }
    void visitPre(DocHtmlDescData *)
    {
      m_t << "<dd>";
    }
    void visitPost(DocHtmlDescData *) 
    {
      m_t << "</dd>\n";
    }
    void visitPre(DocHtmlTable *)
    {
      m_t << "<table border=\"1\" cellspacing=\"3\" cellpadding=\"3\">\n";
    }
    void visitPost(DocHtmlTable *) 
    {
      m_t << "</table>\n";
    }
    void visitPre(DocHtmlRow *)
    {
      m_t << "<tr>\n";
    }
    void visitPost(DocHtmlRow *) 
    {
      m_t << "</tr>\n";
    }
    void visitPre(DocHtmlCell *c)
    {
      if (c->isHeading()) m_t << "<th>"; else m_t << "<td>";
    }
    void visitPost(DocHtmlCell *c) 
    {
      if (c->isHeading()) m_t << "</th>"; else m_t << "</td>";
    }
    void visitPre(DocHtmlCaption *)
    {
      m_t << "<caption align=\"bottom\">";
    }
    void visitPost(DocHtmlCaption *) 
    {
      m_t << "</caption>\n";
    }
    void visitPre(DocIndexEntry *)
    {
      m_hide = TRUE;
    }
    void visitPost(DocIndexEntry *) 
    {
      m_hide = FALSE;
    }
    void visitPre(DocInternal *)
    {
      m_t << "<p><b>" << theTranslator->trForInternalUseOnly() << "</b></p>" << endl;
      m_t << "<p>" << endl;
    }
    void visitPost(DocInternal *) 
    {
      m_t << "</p>" << endl;
    }
    void visitPre(DocHRef *href)
    {
      m_t << "<a href=\"" << href->url() << "\">";
    }
    void visitPost(DocHRef *) 
    {
      m_t << "</a>";
    }
    void visitPre(DocHtmlHeader *header)
    {
      m_t << "<h" << header->level() << ">";
    }
    void visitPost(DocHtmlHeader *header) 
    {
      m_t << "</h" << header->level() << ">\n";
    }
    void visitPre(DocImage *img)
    {
      if (img->type()==DocImage::Html)
      {
        QCString baseName=img->name();
        int i;
        if ((i=baseName.findRev('/'))!=-1 || (i=baseName.findRev('\\'))!=-1)
        {
          baseName=baseName.right(baseName.length()-i-1);
        }
        m_t << "<div align=\"center\">" << endl;
        m_t << "<img src=\"" << img->name() << "\" alt=\"" 
            << baseName << "\">" << endl;
        if (img->hasCaption())
        {
          m_t << "<p><strong>";
        }
      }
      else // other format -> skip
      {
        m_hide=TRUE;
      }
    }
    void visitPost(DocImage *img) 
    {
      if (img->type()==DocImage::Html)
      {
        if (img->hasCaption())
        {
          m_t << "</strong></p>";
        }
        m_t << "</div>" << endl;
      }
      else // other format
      {
        m_hide=FALSE;
      }
    }
    void visitPre(DocDotFile *df)
    {
      QCString baseName=df->file();
      int i;
      if ((i=baseName.findRev('/'))!=-1)
      {
        baseName=baseName.right(baseName.length()-i-1);
      } 
      QCString outDir = Config_getString("HTML_OUTPUT");
      writeDotGraphFromFile(df->file(),outDir,baseName,BITMAP);
      m_t << "<div align=\"center\">" << endl;
      m_t << "<img src=\"" << baseName << "." 
        << Config_getEnum("DOT_IMAGE_FORMAT") << "\" alt=\""
        << baseName << "\">" << endl;
      if (df->hasCaption())
      { 
        m_t << "<p><strong>";
      }
    }
    void visitPost(DocDotFile *df) 
    {
      if (df->hasCaption())
      {
        m_t << "</strong></p>" << endl;
      }
      m_t << "</div>" << endl;
    }
    void visitPre(DocLink *lnk)
    {
      startLink(lnk->ref(),lnk->file(),lnk->anchor());
    }
    void visitPost(DocLink *) 
    {
      endLink();
    }
    void visitPre(DocRef *ref)
    {
      startLink(ref->ref(),ref->file(),ref->anchor());
      if (!ref->hasLinkText()) filter(ref->targetTitle());
    }
    void visitPost(DocRef *) 
    {
      endLink();
      m_t << " ";
    }
    void visitPre(DocSecRefItem *ref)
    {
      QCString refName=ref->file();
      if (refName.right(Doxygen::htmlFileExtension.length())!=Doxygen::htmlFileExtension)
      {
        refName+=Doxygen::htmlFileExtension;
      }
      m_t << "<li><a href=\"" << refName << "#" << ref->anchor() << "\">";

    }
    void visitPost(DocSecRefItem *) 
    {
      m_t << "</a> ";
    }
    void visitPre(DocSecRefList *)
    {
      m_t << "<multicol cols=3>" << endl;
      m_t << "<ul>" << endl;
    }
    void visitPost(DocSecRefList *) 
    {
      m_t << "</ul>" << endl;
      m_t << "</multicol>" << endl;
    }
    void visitPre(DocLanguage *)
    {
    }
    void visitPost(DocLanguage *) 
    {
    }
    void visitPre(DocParamSect *s)
    {
      m_t << "<dl compact><dt><b>";
      switch(s->type())
      {
	case DocParamSect::Param: 
          m_t << theTranslator->trParameters(); break;
	case DocParamSect::RetVal: 
          m_t << theTranslator->trReturnValues(); break;
	case DocParamSect::Exception: 
          m_t << theTranslator->trExceptions(); break;
        default:
          ASSERT(0);
      }
      m_t << ":";
      m_t << "</b></dt><dd>" << endl;
      m_t << "  <table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">" << endl;
    }
    void visitPost(DocParamSect *)
    {
      m_t << "  </table>" << endl;
      m_t << "</dl>" << endl;
    }
    void visitPre(DocParamList *pl)
    {
      m_t << "    <tr><td valign=top><em>";
      QStrListIterator li(pl->parameters());
      const char *s;
      bool first=TRUE;
      for (li.toFirst();(s=li.current());++li)
      {
        if (!first) m_t << ","; else first=FALSE;
        m_t << s;
      }
      m_t << "</em>&nbsp;</td><td>";
    }
    void visitPost(DocParamList *)
    {
      m_t << "</td></tr>" << endl;
    }
    void visitPre(DocXRefItem *x)
    {
      m_t << "<dl compact><dt><b><a class=\"el\" href=\"" 
          << x->file() << Doxygen::htmlFileExtension << "#" << x->anchor() << "\">";
      filter(x->title());
      m_t << ":</a></b></dt><dd>";
    }
    void visitPost(DocXRefItem *)
    {
      m_t << "</dd></dl>" << endl;
    }
    void visitPre(DocInternalRef *ref)
    {
      startLink(0,ref->file(),ref->anchor());
    }
    void visitPost(DocInternalRef *) 
    {
      endLink();
      m_t << " ";
    }
    void visitPre(DocCopy *)
    {
    }
    void visitPost(DocCopy *)
    {
    }

  private:

    //--------------------------------------
    // helper functions 
    //--------------------------------------
    
    void filter(const char *str);
    void startLink(const QCString &ref,const QCString &file,
                   const QCString &anchor);
    void endLink();

    //--------------------------------------
    // state variables
    //--------------------------------------

    QTextStream &m_t;
    BaseCodeDocInterface &m_ci;
    bool m_insidePre;
    bool m_hide;
};

#endif