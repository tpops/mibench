/* Copyright (C) 1989, 1995, 1996 Aladdin Enterprises.  All rights reserved.
  
  This file is part of Aladdin Ghostscript.
  
  Aladdin Ghostscript is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the Aladdin Ghostscript Free Public
  License (the "License") for full details.
  
  Every copy of Aladdin Ghostscript must include a copy of the License,
  normally in a plain ASCII text file named PUBLIC.  The License grants you
  the right to copy, modify and redistribute Aladdin Ghostscript, but only
  under certain conditions described in the License.  Among other things, the
  License requires that the copyright notice and this notice be preserved on
  all copies.
*/

/* idebug.c */
/* Debugging support for Ghostscript interpreter */
/* This file must always be compiled with DEBUG set. */
#undef DEBUG
#define DEBUG
#include "string_.h"
#include "ghost.h"
#include "ialloc.h"		/* for imemory for getting struct type */
#include "idebug.h"		/* defines interface */
#include "idict.h"
#include "iname.h"
#include "ipacked.h"
#include "istack.h"
#include "iutil.h"
#include "ivmspace.h"
#include "ostack.h"			/* for opdef.h */
#include "opdef.h"
#include "store.h"		/* for make_oper for opdef.h */

/* Table of type name strings */
static const char *type_strings[] = { type_print_strings };

/* First unassigned type index */
extern const int tx_next_index;		/* in interp.c */

/* Print a name. */
void
debug_print_name(const ref *pnref)
{	ref sref;
	name_string_ref(pnref, &sref);
	debug_print_string(sref.value.const_bytes, r_size(&sref));
}

/* Print a ref. */
private void
debug_print_full_ref(const ref *pref)
{	unsigned size = r_size(pref);
	ref nref;
	qprintf1("(%x)", r_type_attrs(pref));
	switch ( r_type(pref) )
	   {
	case t_array:
	  qprintf2("array(%u)0x%lx", size, (ulong)pref->value.refs); break;
	case t_astruct:
	  goto strct;
	case t_boolean:
	  qprintf1("boolean %x", pref->value.boolval); break;
	case t_device:
	  qprintf1("device 0x%lx", (ulong)pref->value.pdevice); break;
	case t_dictionary:
	  qprintf3("dict(%u/%u)0x%lx",
		   dict_length(pref), dict_maxlength(pref),
		   (ulong)pref->value.pdict);
	  break;
	case t_file:
	  qprintf1("file 0x%lx", (ulong)pref->value.pfile); break;
	case t_fontID:
	  goto strct;
	case t_integer:
	  qprintf1("int %ld", pref->value.intval); break;
	case t_mark:
	  qprintf("mark"); break;
	case t_mixedarray:
	  qprintf2("mixed packedarray(%u)0x%lx", size,
		   (ulong)pref->value.packed); break;
	case t_name:
	  qprintf2("name(0x%lx#%u)", (ulong)pref->value.pname,
		   name_index(pref));
	  debug_print_name(pref);
	  break;
	case t_null:
	  qprintf("null"); break;
	case t_oparray:
	  qprintf2("op_array(%u)0x%lx:", size, (ulong)pref->value.const_refs);
	  { const op_array_table *opt = op_index_op_array_table(size);
	    name_index_ref(opt->nx_table[size - opt->base_index], &nref);
	  }
	  debug_print_name(&nref);
	  break;
	case t_operator:
	  qprintf1("op(%u", size);
	  if ( size > 0 && size < op_def_count ) /* just in case */
	    qprintf1(":%s", (const char *)(op_def_table[size]->oname + 1));
	  qprintf1(")0x%lx", (ulong)pref->value.opproc);
	  break;
	case t_real:
	  qprintf1("real %f", pref->value.realval); break;
	case t_save:
	  qprintf1("save %lu", pref->value.saveid); break;
	case t_shortarray:
	  qprintf2("short packedarray(%u)0x%lx", size,
		   (ulong)pref->value.packed); break;
	case t_string:
	  qprintf2("string(%u)0x%lx", size, (ulong)pref->value.bytes); break;
	case t_struct:
strct:	  { obj_header_t *obj = (obj_header_t *)pref->value.pstruct;
	    qprintf2("struct %s 0x%lx",
		     (r_is_foreign(pref) ? "-foreign-" :
		      gs_struct_type_name_string(gs_object_type(imemory, obj))),
		     (ulong)obj);
	  }
	  break;
	default: qprintf1("type 0x%x", r_type(pref));
	   }
}
private void
debug_print_packed_ref(const ref_packed *pref)
{	ushort elt = *pref & packed_value_mask;
	ref nref;
	switch ( *pref >> r_packed_type_shift )
	{
	case pt_executable_operator:
	  qprintf("<op_name>");
	  op_index_ref(elt, &nref);
	  debug_print_ref(&nref);
	  break;
	case pt_integer:
	  qprintf1("<int> %d", (int)elt + packed_min_intval);
	  break;
	case pt_literal_name:
	  qprintf("<lit_name>"); goto ptn;
	case pt_executable_name:
	  qprintf("<exec_name>");
ptn:	  name_index_ref(elt, &nref);
	  qprintf2("(0x%lx#%u)", (ulong)nref.value.pname, elt);
	  debug_print_name(&nref);
	  break;
	default:
	  qprintf2("<packed_%d?>0x%x", *pref >> r_packed_type_shift, elt);
	}
}
void
debug_print_ref(const ref *pref)
{	if ( r_is_packed(pref) )
		debug_print_packed_ref((const ref_packed *)pref);
	else
		debug_print_full_ref(pref);
	fflush(dstderr);
}

/* Dump one ref. */
void
debug_dump_one_ref(const ref *p)
{	uint attrs = r_type_attrs(p);
	uint type = r_type(p);
	static const attr_print_mask apm[] = {attr_print_masks, {0,0,0}};
	const attr_print_mask *ap = apm;
#define buf_size 30
	char buf[buf_size + 1];
	uint plen;

	if ( type >= tx_next_index )
	  qprintf1("0x%02x?? ", type);
	else if ( type >= t_next_index )
	  qprintf("opr* ");
	else
	  qprintf1("%s ", type_strings[type]);
	for ( ; ap->mask; ++ap )
	  if ( (attrs & ap->mask) == ap->value )
	    dputc(ap->print);
	qprintf2(" 0x%04x 0x%08lx", r_size(p), *(const ulong *)&p->value);
	if ( obj_cvs(p, (byte *)buf, countof(buf) - 1, &plen, NULL) >= 0 &&
	     ((buf[plen] = 0), strcmp(buf, "--nostringval--"))
	   )
	  qprintf1(" = %s", buf);
	fflush(dstderr);
}

/* Dump a region of memory containing refs. */
void
debug_dump_refs(const ref *from, uint size, const char *msg)
{	const ref *p = from;
	uint count = size;
	if ( size && msg )
		qprintf2("%s at 0x%lx:\n", msg, (ulong)from);
	while ( count-- )
	  {	/* The double cast in the next line is to pacify some */
		/* unreasonably picky compilers. */
		qprintf2("..%04x: 0x%04x ", (uint)(ulong)p & 0xffff,
			 r_type_attrs(p));
		debug_dump_one_ref(p);
		dputc('\n');
		p++;
	  }
}

/* Dump a stack. */
void
debug_dump_stack(const ref_stack *pstack, const char *msg)
{	uint i;
	const char *m = msg;
	for ( i = ref_stack_count(pstack); i != 0; )
	  { const ref *p = ref_stack_index(pstack, --i);
	    if ( m )
	      qprintf2("%s at 0x%lx:\n", m, (ulong)pstack),
	      m = NULL;
	    qprintf2("0x%lx: 0x%02x ", (ulong)p, r_type(p));
	    debug_dump_one_ref(p);
	    dputc('\n');
	  }
}

/* Dump an array. */
void
debug_dump_array(const ref *array)
{	const ref_packed *pp;
	unsigned int type = r_type(array);
	uint len;

	switch (type)
	   {
	default:
		qprintf2 ("%s at 0x%lx isn't an array.\n",
			  (type < countof(type_strings) ?
			   type_strings[type] : "????"),
			  (ulong)array);
		return;
	case t_oparray:
		/* This isn't really an array, but we'd like to see */
		/* its contents anyway. */
		debug_dump_array(array->value.const_refs);
		return;
	case t_array:
	case t_mixedarray:
	case t_shortarray: 
		;
	   }

	/* This "packed" loop works for all array-types. */
	for ( len = r_size (array), pp = array->value.packed;
	      len > 0;
	      len--, pp = packed_next(pp))
	   {	ref temp;
		packed_get(pp, &temp);
		/* The double cast in the next line is to pacify some */
		/* unreasonably picky compilers. */
		qprintf3("..%04x%c 0x%02x ", 
			 (uint)(ulong)pp & 0xffff,
			 ((r_is_packed(pp)) ? '*' : ':'),
			 r_type(&temp));
		debug_dump_one_ref(&temp);
		dputc ('\n');
	   }
}
