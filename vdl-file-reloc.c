#include "vdl-file-reloc.h"
#include "machine.h"
#include "vdl-log.h"

static ElfW(Vernaux) *
sym_to_vernaux (unsigned long index,
		ElfW(Half) *dt_versym, 
		ElfW(Verneed) *dt_verneed,
		unsigned long dt_verneednum)
{
  if (dt_versym != 0 && dt_verneed != 0 && dt_verneednum != 0)
    {
      // the same offset used to look in the symbol table (dt_symtab)
      // is an offset in the version table (dt_versym).
      // dt_versym contains a set of 15bit indexes and 
      // 1bit flags packed into 16 bits. When the upper bit is
      // set, the associated symbol is 'hidden', that is, it
      // cannot be referenced from outside of the object.
      ElfW(Half) ver_ndx = dt_versym[index];
      if (ver_ndx & 0x8000)
	{
	  return 0;
	}
      // search the version needed whose vd_ndx is equal to ver_ndx.
      ElfW(Verneed) *cur, *prev;
      for (cur = dt_verneed, prev = 0; 
	   cur != prev; 
	   prev = cur, cur = (ElfW(Verneed) *)(((unsigned long)cur)+cur->vn_next))
	{
	  VDL_LOG_ASSERT (cur->vn_version == 1, "version number invalid for Verneed");
	  ElfW(Vernaux) *cur_aux, *prev_aux;
	  for (cur_aux = (ElfW(Vernaux)*)(((unsigned long)cur)+cur->vn_aux), prev_aux = 0;
	       cur_aux != prev_aux; 
	       prev_aux = cur_aux, cur_aux = (ElfW(Vernaux)*)(((unsigned long)cur_aux)+cur_aux->vna_next))
	    {
	      if (cur_aux->vna_other == ver_ndx)
		{
		  return cur_aux;
		}
	    }
	}
    }
  return 0;
}

static void
vdl_file_reloc_plt (struct VdlFile *file)
{
  VDL_LOG_FUNCTION ("file=%s", file->name);
  ElfW(Rel) *dt_jmprel = (ElfW(Rel)*)vdl_file_get_dynamic_p (file, DT_JMPREL);
  unsigned long dt_pltrel = vdl_file_get_dynamic_v (file, DT_PLTREL);
  unsigned long dt_pltrelsz = vdl_file_get_dynamic_v (file, DT_PLTRELSZ);
  const char *dt_strtab = (const char *)vdl_file_get_dynamic_p (file, DT_STRTAB);
  ElfW(Sym) *dt_symtab = (ElfW(Sym)*)vdl_file_get_dynamic_p (file, DT_SYMTAB);
  ElfW(Half) *dt_versym = (ElfW(Half)*)vdl_file_get_dynamic_p (file, DT_VERSYM);
  ElfW(Verneed) *dt_verneed = (ElfW(Verneed)*)vdl_file_get_dynamic_p (file, DT_VERNEED);
  unsigned long dt_verneednum = vdl_file_get_dynamic_v (file, DT_VERNEEDNUM);
  
  if (dt_pltrel != DT_REL || dt_pltrelsz == 0 || 
      dt_jmprel == 0 || dt_strtab == 0 || 
      dt_symtab == 0)
    {
      return;
    }
  int i;
  for (i = 0; i < dt_pltrelsz/sizeof(ElfW(Rel)); i++)
    {
      ElfW(Rel) *rel = &dt_jmprel[i];
      ElfW(Sym) *sym = &dt_symtab[ELFW_R_SYM (rel->r_info)];
      ElfW(Vernaux) *ver = sym_to_vernaux (ELFW_R_SYM (rel->r_info),
					   dt_versym, dt_verneed, dt_verneednum);
      const char *symbol_name;
      if (sym->st_name == 0)
	{
	  symbol_name = 0;
	}
      else
	{
	  symbol_name = dt_strtab + sym->st_name;
	}
      machine_perform_relocation (file, rel, sym, ver, symbol_name);
    }
}
static void
vdl_file_lazy_reloc_plt (struct VdlFile *file)
{
  VDL_LOG_FUNCTION ("file=%s", file->name);
  ElfW(Rel) *dt_jmprel = (ElfW(Rel)*)vdl_file_get_dynamic_p (file, DT_JMPREL);
  unsigned long dt_pltrel = vdl_file_get_dynamic_v (file, DT_PLTREL);
  unsigned long dt_pltrelsz = vdl_file_get_dynamic_v (file, DT_PLTRELSZ);
  
  if (dt_pltrel != DT_REL || dt_pltrelsz == 0 || 
      dt_jmprel == 0)
    {
      return;
    }
  int i;
  for (i = 0; i < dt_pltrelsz/sizeof(ElfW(Rel)); i++)
    {
      ElfW(Rel) *rel = &dt_jmprel[i];
      unsigned long *reloc_addr = (unsigned long*) (rel->r_offset + file->load_base);
      *reloc_addr += file->load_base;
    }
}
unsigned long vdl_file_reloc_one_plt (struct VdlFile *file, 
				      unsigned long offset)
{
  unsigned long dt_jmprel = vdl_file_get_dynamic_p (file, DT_JMPREL);
  unsigned long dt_pltrel = vdl_file_get_dynamic_v (file, DT_PLTREL);
  unsigned long dt_pltrelsz = vdl_file_get_dynamic_v (file, DT_PLTRELSZ);
  const char *dt_strtab = (const char *)vdl_file_get_dynamic_p (file, DT_STRTAB);
  ElfW(Sym) *dt_symtab = (ElfW(Sym)*)vdl_file_get_dynamic_p (file, DT_SYMTAB);
  ElfW(Half) *dt_versym = (ElfW(Half)*)vdl_file_get_dynamic_p (file, DT_VERSYM);
  ElfW(Verneed) *dt_verneed = (ElfW(Verneed)*)vdl_file_get_dynamic_p (file, DT_VERNEED);
  unsigned long dt_verneednum = vdl_file_get_dynamic_v (file, DT_VERNEEDNUM);
  
  if (dt_pltrel != DT_REL || dt_pltrelsz == 0 || 
      dt_jmprel == 0 || dt_strtab == 0 || 
      dt_symtab == 0)
    {
      return 0;
    }
  ElfW(Rel) *rel = (ElfW(Rel)*)(dt_jmprel+offset);
  ElfW(Sym) *sym = &dt_symtab[ELFW_R_SYM (rel->r_info)];
  ElfW(Vernaux) *ver = sym_to_vernaux (ELFW_R_SYM (rel->r_info),
				       dt_versym, dt_verneed, dt_verneednum);
  const char *symbol_name;
  if (sym->st_name == 0)
    {
      symbol_name = 0;
    }
  else
    {
      symbol_name = dt_strtab + sym->st_name;
    }
  unsigned long symbol = machine_perform_relocation (file, rel, sym, ver, symbol_name);
  return symbol;
}


void
vdl_file_reloc_got (struct VdlFile *file)
{
  VDL_LOG_FUNCTION ("file=%s", file->name);
  ElfW(Rel) *dt_rel = (ElfW(Rel)*)vdl_file_get_dynamic_p (file, DT_REL);
  unsigned long dt_relsz = vdl_file_get_dynamic_v (file, DT_RELSZ);
  unsigned long dt_relent = vdl_file_get_dynamic_v (file, DT_RELENT);
  const char *dt_strtab = (const char *)vdl_file_get_dynamic_p (file, DT_STRTAB);
  ElfW(Sym) *dt_symtab = (ElfW(Sym)*)vdl_file_get_dynamic_p (file, DT_SYMTAB);
  ElfW(Half) *dt_versym = (ElfW(Half)*)vdl_file_get_dynamic_p (file, DT_VERSYM);
  ElfW(Verneed) *dt_verneed = (ElfW(Verneed)*)vdl_file_get_dynamic_p (file, DT_VERNEED);
  unsigned long dt_verneednum = vdl_file_get_dynamic_v (file, DT_VERNEEDNUM);
  if (dt_rel == 0 || dt_relsz == 0 || dt_relent == 0 ||
      dt_strtab == 0 || dt_symtab == 0)
    {
      return;
    }
  uint32_t i;
  for (i = 0; i < dt_relsz/dt_relent; i++)
    {
      ElfW(Rel) *rel = &dt_rel[i];
      ElfW(Sym) *sym = &dt_symtab[ELFW_R_SYM (rel->r_info)];
      ElfW(Vernaux) *ver = sym_to_vernaux (ELFW_R_SYM (rel->r_info),
					   dt_versym, dt_verneed, dt_verneednum);
      const char *symbol_name;
      if (sym->st_name == 0)
	{
	  symbol_name = 0;
	}
      else
	{
	  symbol_name = dt_strtab + sym->st_name;
	}
      machine_perform_relocation (file, rel, sym, ver, symbol_name);
    }
}

void vdl_file_reloc (struct VdlFile *file, int now)
{
  if (file->reloced)
    {
      return;
    }
  file->reloced = 1;

  // relocate dependencies first:
  struct VdlFileList *cur;
  for (cur = file->deps; cur != 0; cur = cur->next)
    {
      vdl_file_reloc (cur->item, now);
    }

  vdl_file_reloc_got (file);
  if (now)
    {
      // perform full PLT relocs _now_
      vdl_file_reloc_plt (file);
    }
  else
    {
      // perform simple PLT reloc to make each
      // associated GOT entry point to the right
      // jmp initially
      vdl_file_lazy_reloc_plt (file);
      // then, make sure the loader gets called
      // whenever the code jumps into a PLT entry.
      machine_lazy_setup (file);
    }
}