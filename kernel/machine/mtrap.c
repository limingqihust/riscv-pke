#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include "kernel/elf.h"
#include "util/string.h"
static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}


void print_mtrap_type()
{
  uint64 mcause=read_csr(mcause);
  switch(mcause)
  {
    case CAUSE_MTIMER:
      sprint("cause_mtimer\n");
      break;
    case CAUSE_FETCH_ACCESS:
      sprint("cause_fetch_access\n");
      break;
    case CAUSE_LOAD_ACCESS:
      sprint("cause_load_access\n");
      break;
    case CAUSE_STORE_ACCESS:
      sprint("cause_store_access\n");
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      sprint("cause_illegal instruction\n");
      break;
    case CAUSE_MISALIGNED_LOAD:
      sprint("cause_misaligned_load\n");
      break;
    case CAUSE_MISALIGNED_STORE:
      sprint("cause_misaligned_store\n");
      break;
    default:
      sprint("other type trap\n");
      break;
  }
}


/*
typedef struct {
    uint64 dir; 
    char *file;
} code_file;
typedef struct {
    uint64 addr, line, file;
} addr_line;
*/
void print_errorline(uint64 mepc)
{
  char *debugline=current->debugline; 
  char **dir=current->dir;            // ���д����ļ����ļ������Ƶ��ַ������� ��64��
  code_file *file=current->file;      // ���д����ļ��� {�ļ����ַ���ָ��,���ļ���·����dir�����е�����} ��64��
  addr_line *line=current->line;      // {ָ���ַ�������кţ��ļ�����file�����е�����}
  int line_ind=current->line_ind;
  char* file_name=NULL;
  char* dir_name=NULL;
  uint64 row_num=0;

  /*�ҵ������쳣�Ĵ�����к� �ļ��� �ļ�Ŀ¼��*/
  for(int i=0;i<line_ind;i++)
  {
    if(mepc==line[i].addr)
    {
      row_num=line[i].line;
      file_name=file[line[i].file].file;
      dir_name=dir[file[line[i].file].dir];
      break;
    }
  }
  sprint("Runtime error at %s/%s:%lld\n",dir_name,file_name,row_num);

  /*�����ļ������ļ�Ŀ¼������c�ļ��ľ���·����*/
  int c_size=strlen(file_name)+strlen(dir_name)+1;
  char c_name[c_size];
  strcpy(c_name,dir_name);
  c_name[strlen(dir_name)]='/';
  strcpy(c_name+strlen(dir_name)+1,file_name);
  c_name[c_size+1]='\0';
  // sprint("c file absolute path:%s\n",c_name);


  /*�򿪴����쳣�Ĵ����ļ� �����к��������*/
  elf_info info;
  spike_file_t* f = spike_file_open(c_name, O_RDONLY, 0);
  // sprint("%d %d\n",f->kfd,f->refcnt);
  int cur_row=1;
  char cur_char;
  int cur_idx=0;

  char c_code[1024];
  spike_file_read(f, (void*)c_code,1024);   // ���ﴥ�����쳣������ 
  for(cur_idx=0;;cur_idx++)
  {
    if(c_code[cur_idx]=='\n')
      cur_row++;
    if(cur_row==row_num)
      break;
  }
  cur_idx++;
  for(;;cur_idx++)
  {
    sprint("%c",c_code[cur_idx]);
    if(c_code[cur_idx]=='\n')
      break;
  }

  // /*���������쳣�Ĵ�����*/
  // for(cur_idx=0;;cur_idx++)
  // {
  //   spike_file_pread(f, (void*)&cur_char, 1, cur_idx);   // ���ﴥ�����쳣������ 
  //   // panic("here\n");
  //   if(cur_char=='\n')
  //     cur_row++;
  //   if(cur_row==row_num)
  //     break;
  // }
  // /*��������쳣�Ĵ�����*/
  // for(int i=0;;i++)
  // {
  //   spike_file_pread(f, (void*)&cur_char,1,cur_idx+i+1);
  //   sprint("%c",cur_char);
  //   if(cur_char=='\n')
  //     break;
  // }

}


//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  uint64 mepc=read_csr(mepc);
  // print_mtrap_type();
  print_errorline(mepc);
  switch (mcause) {
    case CAUSE_MTIMER:
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      handle_load_access_fault();
      break;
    case CAUSE_STORE_ACCESS:
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      handle_illegal_instruction();
      break;
    case CAUSE_MISALIGNED_LOAD:
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      handle_misaligned_store();
      break;

    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
