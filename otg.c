#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdckdint.h>
#include <string.h>

//-----------------------------------------------------------------------------------------
//--------------------------------- BEG TYPEDEFS, MACROS ----------------------------------

typedef long int pos_t;
#define POS	"l"

typedef int_least16_t imm_t ;
#define PRIimm PRIdLEAST16 
#define SCNimm SCNdLEAST16 
typedef uint_least16_t uimm_t;
#define PRIuimm PRIuLEAST16 
#define SCNuimm SCNuLEAST16 

typedef int_fast32_t regv_t;
#define PRIregv PRIdFAST32 
#define SCNregv SCNiFAST32 
#define REGV_WIDTH 32
#define REGV_AMIN INT_FAST32_MIN
#define REGV_AMAX INT_FAST32_MAX

#define MIN_REGV ((regv_t)-2147483648) 
#define MAX_REGV ((regv_t)2147483647)

typedef uint_fast32_t uregv_t;
#define SCNuregv SCNuFAST32 
#define PRIuregv PRIuFAST32 
#define UREGV_AMAX UINT_FAST32_MAX
#define UREGV_AMIN UINT_FAST32_MIN
#define MIN_UREGV ((uregv_t)0) 
#define MAX_UREGV ((uregv_t)4294967295)

#define STR_IMPL(X) #X
#define STR(X) STR_IMPL(X)

#define SPACING 1

//#define OPT
#ifdef OPT
	#define UNREACHABLE unreachable()
#else
	#define UNREACHABLE assert(0)
#endif

#define MIN_OPC_LEN 2
#define MAX_OPC_LEN 4
#define MAX_OPC_SIZE (MAX_OPC_LEN + 1)
#define OPC_READ_BUF_LEN 5
static_assert(MAX_OPC_LEN + 1 == OPC_READ_BUF_LEN);
#define OPC_READ_BUF_LEN_STR STR(OPC_READ_BUF_LEN)
#define OPC_READ_BUF_SIZE (OPC_READ_BUF_LEN + 1)

#define MIN_NUM_OPRS 2
#define MAX_NUM_OPRS 3
#define MAX_NUM_OPRS_TO_READ (MAX_NUM_OPRS + 1)

#define NUM_REG 16
#define MIN_REG 0
#define MAX_REG (NUM_REG - 1)

#define MIN_IMM (-32768)
#define MAX_IMM (32767)

#define DIV_TST    "-D"
#define NO_DIV_TST "-D'"

#define HASH4(A, B, C, D) HASH3(B, C, D)
#define HASH3(A, B, C) HASH2(B, C)
#define HASH2(A, B)	((A) - (B))

#define ADD  "add"
#define SUB  "sub"
#define MUL  "mul"
#define SHL  "shl"
#define SHR  "shr"
#define BOR  "bor"
#define BAND "band"
#define LI   "li"
#define JZ   "jz"
#define JP   "jp"

#define MAX_DBG_CMD 8
#define MAX_DBG_CMD_TO_READ 9
static_assert(MAX_DBG_CMD_TO_READ > MAX_DBG_CMD);
#define DBG_CMD_BUF_SIZE (MAX_DBG_CMD_TO_READ + 1)
#define MAX_DBG_CMD_TO_READ_STR STR(MAX_DBG_CMD_TO_READ)

//--------------------------------- END TYPEDEFS, MACROS -----------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG ENUM ----------------------------------------

enum cmd_ln_arg_t {
	cmd_ln_arg_prog_name = 0,
	cmd_ln_arg_file_name = 1,
	cmd_ln_arg_conc_div_test = 2,
	min_num_cmd_args,
	max_num_cmd_args = min_num_cmd_args + NUM_REG
};

enum err_t : int {
	err_usr,
	err_stx,
	err_int,
	err_eof,
	err_eol,
	err_ok,
	err_other
};
enum goto_nxt_ln_t : int{
	goto_nxt_ln_eof	  = err_eof,
	goto_nxt_ln_err	  = err_int,
	goto_nxt_ln_ok    = err_ok,
	goto_nxt_ln_eol_eof = err_other
};
enum chk_fscanf_t : int {
	chk_fscanf_eof = err_eof,
	chk_fscanf_asn = err_other,
	chk_fscanf_err = err_int,
	chk_fscanf_ok  = err_ok
};
enum get_t : int {
	get_eof = err_eof,
	get_err	= err_int,
	get_ok	= err_ok
};
enum getn_t : int {
	getn_err  = err_int,
	getn_eof  = err_eof,	//note this corresponds to EOF AND No input Read!
	getn_ok   = err_ok 
};
enum go_ovr_nl_t : int {
	go_ovr_nl_eof 	= err_eof,
	go_ovr_nl_err 	= err_int,
	go_ovr_nl_ok  	= err_ok,
	go_ovr_nl_non_nl	= err_other
};
enum get_opc_t : int {
	get_opc_int_err = err_int,
	get_opc_stx_err = err_stx,
	get_opc_eof		= err_eof,
	get_opc_ok	    = err_ok
};
enum get_opr_t : int {
	get_opr_int_err	= err_int,
	get_opr_stx_err = err_stx,
	get_opr_eof		= err_eof,
	get_opr_eol		= err_eol,
	get_opr_ok		= err_ok
};
enum get_oprs_t : int {
	get_oprs_int_err	= err_int,
	get_oprs_stx_err    = err_stx,	//counts as opr_too many
	get_oprs_eof		= err_eof,
	get_oprs_eol		= err_eol,
};
enum dec_t : int {
	dec_stx_err = err_stx,
	dec_int_err = err_int,
	dec_ok		= err_ok
};
enum get_ops_t : int {
	get_ops_ok		= err_ok,	
	get_ops_stx_err = err_stx,
	get_ops_usr_err = err_usr,
	get_ops_int_err = err_int
};
enum get_op_t : int {
	get_op_ok		= get_ops_ok,
	get_op_stx_err  = get_ops_stx_err,
	get_op_usr_err	= get_ops_usr_err,
	get_op_int_err  = get_ops_int_err,
	get_op_eof		= err_eof
};
enum hopc_t {
	hopc_add  = HASH3('a', 'd', 'd'),
	hopc_sub  = HASH3('s', 'u', 'b'),
	hopc_mul  = HASH3('m', 'u', 'l'),
	hopc_shl  = HASH3('s', 'h', 'l'),
	hopc_shr  = HASH3('s', 'h', 'r'),
	hopc_bor  = HASH3('b', 'o', 'r'),
	hopc_band = HASH4('b', 'a', 'n', 'd'),
	hopc_li   = HASH2('l', 'i'),
	hopc_jz   = HASH2('j', 'z'),
	hopc_jp   = HASH2('j', 'p')
};
enum opc_t {
	opc_add = 0,
	opc_sub = 1,
	opc_mul = 2,
	opc_shl = 3,
	opc_shr = 4,
	opc_bor = 5,
	opc_band = 6,
	opc_li = 7,
	opc_jz = 8,
	opc_jp = 9
};

enum get_dbg_cmd_t {
	get_dbg_cmd_stx_err,
	get_dbg_cmd_int_err,
	get_dbg_cmd_ok,
	get_dbg_cmd_eof,
};

enum dbg_hash_t {
	dbg_hash_step = 's',
	dbg_hash_continue = 'c',
	dbg_hash_disable = 'd',
	dbg_hash_enable = 'e',
	dbg_hash_print = 'p',
	dbg_hash_break = 'b',
	dbg_hash_run = 'r',
	dbg_hash_kill = 'k',
};

enum dbg_cmd_t {
	dbg_cmd_step,
	dbg_cmd_continue,
	dbg_cmd_disable,
	dbg_cmd_enable,
	dbg_cmd_print,
	dbg_cmd_break,
	dbg_cmd_run,
	dbg_cmd_kill
};

//---------------------------------------- END ENUM ----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG STRUCTS ---------------------------------------
struct dbg_cmd_str_t {
	char s[DBG_CMD_BUF_SIZE];
	size_t n;
};

struct ctx_t {
	const char* filename;
	const char* func;
	long int line;
};
struct file_t{
	FILE* stream;
	const char* name;
	pos_t line;
	pos_t pos;
	long fpi;
	pos_t fpi_pos;
};
struct opc_str_t {
	char s[OPC_READ_BUF_SIZE];
	int n;
};
struct oprs_t {
	imm_t b[MAX_NUM_OPRS_TO_READ]; 
	int n;
};
struct op_t {
	uimm_t t: 4;
	uimm_t d: 4;
	uimm_t a: 4;
	uimm_t b: 4;
	imm_t i; 
};
struct ops_t {
	int n;
	struct op_t* b;
};
//------------------------------------------------------------------------------------------
//-------------------------------------- BEG GLOBALS ---------------------------------------
static const char* const opc_strs[] = {
	[opc_add] = ADD,
	[opc_sub] = SUB,
	[opc_mul] = MUL,
	[opc_shl] = SHL,
	[opc_shr] = SHR,
	[opc_bor] = BOR,
	[opc_band] = BAND,
	[opc_li] = LI,
	[opc_jz] = JZ,
	[opc_jp] = JP
};

struct file_t stdoutf;

//-------------------------------------- END GLOBALS ---------------------------------------
//------------------------------------------------------------------------------------------

//-------------------------------------- END STRUCTS ---------------------------------------
//------------------------------------------------------------------------------------------
static struct ctx_t init_ctx(const char* const filename, const char* const func);
static void set_line(struct ctx_t* const ctx, const long line);
//--------------------------------------- END CONT_T ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------  BEG FILE_T  ---------------------------------------
static bool close(struct file_t* const file);
static bool open(
	struct file_t* const file, 
	const char* const filename, 
	const char* const mode
);
static bool open_asm(struct file_t* const file, char* const buf);
static void inc_line(struct file_t* const file);
static void clr_pos(struct file_t* const file);
static void mov_pos(struct file_t* const file, pos_t off);
static void inc_pos(struct file_t* const file);
static void dec_pos(struct file_t* const file);
static bool set_fpi(struct file_t* const file);
static enum goto_nxt_ln_t goto_nxt_ln(struct file_t* const file);
static pos_t get_lcnt(struct file_t* const file);
static enum chk_fscanf_t chk_fscanf(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int num_asn_exp,
	const int asn
);
static bool seek_fpi(struct file_t* const file);
static void seek_beg(struct file_t* const file);
static enum get_t get(int* const c, struct file_t* const file);
static bool unget(const int c, struct file_t* const file);
static int getn(
	char* const line,
	const int n,
	const struct file_t* const file
);
static char* rdln(struct file_t* const fILE);
static bool rdln_prln(struct file_t* const file);
static enum go_ovr_nl_t go_ovr_chk_get(
	struct ctx_t* const ctx,
	long line,
	enum get_t ret
);
static enum go_ovr_nl_t go_ovr_nl(struct file_t* const file, int* const o_c);
void init_stdoutf(void);
//-------------------------------------  END FILE_T  ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG USR RPT ---------------------------------------
static void rpt_usr_err_pre(void);
static void rpt_few_args(int argc);
static void rpt_mny_args(int argc);
static void rpt_fopen_err(const char* const filename);
static void rpt_regv_not_int(const char* const reg_str, const int reg_num);
static void rpt_inv_regv(const char* reg_str, const int reg_num);
static void rpt_conc_div_tst_many_args(void);
static void rpt_conc_div_tst(const char* flag);
//-------------------------------------- END USR RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG INT RPT ---------------------------------------
static void rpt_err_pre(const char* const type, const struct file_t* const file);
static void rpt_int_err_cont(const struct ctx_t* const ctx);
static void rpt_int_err_cont_if(const struct ctx_t* const ctx, const bool b);
static void rpt_int_err_pre(const struct file_t* const file);
static void rpt_int_err(
	const struct ctx_t* const ctx,
	const struct file_t* const file,
	const char* const descr
);
static void rpt_setvbuf_err(
	const struct ctx_t* const ctx,
	const struct file_t* const file,
	const char* const buf,
	const int mode,
	const size_t buf_sz
);
static void rpt_fscanf_err(const struct ctx_t* const ctx, const struct file_t* const file);
static void rpt_fscanf_unexp_eof(
	const struct ctx_t* const ctx, 
	const struct file_t* const file
);
static void rpt_fscanf_asn_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int exp,
	const int act
);
static void rpt_fseek_err(const struct ctx_t* const ctx, const struct file_t* const file);
static void rpt_ftell_err(const struct ctx_t* const ctx, const struct file_t* const file);
static void rpt_malloc_err(
	const struct ctx_t* const ctx, 
	const size_t n
);
static void rpt_line_malloc_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const pos_t n
);
static void rpt_ungetc_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int c
);
static void rpt_getc_err(const struct ctx_t* const ctx, const struct file_t* const file);
static void rpt_fgets_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int n
);
static void rpt_fclose(const struct ctx_t* const ctx, const struct file_t* const file);
//-------------------------------------- END INT RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG STX RPT ---------------------------------------
static void rpt_stx_err_pre(struct file_t* const file);
static bool rdln_prln_stx_err(struct file_t* const file);
static bool rpt_indent(struct file_t* const file);
static bool rpt_long_opc(struct file_t* const file, const struct opc_str_t* const opc);
static bool rpt_few_oprs(struct file_t* const file, const struct oprs_t* const oprs);
static bool rpt_shrt_opc(struct file_t* const file, const struct opc_str_t* const opc);
static bool rpt_not_int(struct file_t* const file, const int c);
static bool rpt_large_opr(struct file_t* const file);
static bool rpt_spc(struct file_t* const file, const pos_t n);
static bool rpt_high_opr_num(struct file_t* const file);
static bool rpt_not_opc(struct file_t* const file, const struct opc_str_t* const opc);
static bool rpt_inv_opr_num(
	struct file_t* const file, 
	const struct opc_str_t* const opc,
	const struct oprs_t* const oprs,
	const int exp
);
static bool rpt_inv_reg(
	struct file_t* const file, 
	const struct oprs_t* const oprs,
	const int opr_num
);
static bool rpt_inv_imm(
	struct file_t* const file, 
	const struct oprs_t* const oprs,
	const int opr_num
);
//-------------------------------------- END STX RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG RNT RPT ---------------------------------------
static void rpt_rnt_err_pre(void);
static void rpt_op_pre(const pos_t ip);
static void rpt_regs(const regv_t* const regs);
static void rpt_cyc(long long cyc);
static void rpt_regv(const int r, const regv_t v);
static void rpt_reg3_op(const struct op_t op, const pos_t ip);
static void rpt_reg2_op(const struct op_t op, const pos_t ip);
static void rpt_reg_imm_op(const struct op_t op, const pos_t ip);
static void rpt_ovf(const struct op_t op, const pos_t ip);
static void rpt_inv_jmp(const struct op_t op, const pos_t ip, const pos_t bad_ip);
static void rpt_add(const struct op_t op, const pos_t ip);
static void rpt_sub(const struct op_t op, const pos_t ip);
static void rpt_mul(const struct op_t op, const pos_t ip);
static void rpt_shl(const struct op_t op, const pos_t ip);
static void rpt_shft_amt(const struct op_t op, const pos_t ip);
static void rpt_jz(const struct op_t op, const pos_t ip, const pos_t bad_ip);
static void rpt_jp(const struct op_t op, const pos_t ip, const pos_t bad_ip);
//-------------------------------------- END RNT RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------- BEG ERR PROP ---------------------------------------
static int prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ,
	const int succ_v,
	const int err_v
);
static bool prop_bool_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ
);
//------------------------------------- END ERR PROP ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------- BEG OPC STR  ---------------------------------------
static enum get_opc_t opc_prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool stx_err
);
static enum get_opc_t get_opc(struct file_t* const file,struct opc_str_t* const opc);
//------------------------------------- END OPC STR  ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG OPR -----------------------------------------
static enum get_opr_t opr_prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ,
	const enum get_opr_t succ_v,
	const enum get_opr_t file_v
);
static enum get_opr_t opr_chk_fscanf(
	struct ctx_t* const ctx, 
	const struct file_t* const file,
	const long line,
	const int num_asn_exp,
	const int asn
);
static bool is_opr_chr(const int c);
static enum get_opr_t get_opr(struct file_t* const file, imm_t* const opr);
static enum get_oprs_t get_oprs(struct file_t* const file, struct oprs_t* const oprs);
//---------------------------------------- END OPR -----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG DEC -----------------------------------------
static int hash(const struct opc_str_t* const opc);
static enum dec_t dec_prop_err(struct ctx_t* const, const long line, const bool succ);
static bool is_opr_btw(imm_t opr, int lb, int ub);
static bool is_reg(imm_t reg);
static bool is_imm(imm_t imm);
static enum dec_t dec_rpt_int_err_cont_if(
	const struct ctx_t* const ctx, 
	const enum dec_t ret
);
static enum dec_t dec(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs,
	struct op_t* const op
);
static enum dec_t dec_reg_op(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs, 
	struct op_t* const op,
	const int num_regs
);
static enum dec_t chk_opr_num(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs, 
	const int num_regs
);
static enum dec_t chk_reg_oprs(
	struct file_t* const file,
	const struct oprs_t* const oprs
);
//---------------------------------------- END DEC -----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG REGV ----------------------------------------
static bool init_reg(regv_t* const reg, const char* const reg_str, const int i);
static bool init_regs(regv_t* const regs, int n, const char* const reg_strs[n]);
//---------------------------------------- END REGV ----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG GET OPS ---------------------------------------
static enum get_op_t get_op(struct file_t* const file, struct op_t* const op);
static enum get_ops_t get_ops(const char* const filename, struct ops_t* const ops);
//-------------------------------------- END GET OPS ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//--------------------------------------- BEG DEBUG ----------------------------------------
int dbg_hash(const struct dbg_cmd_str_t* const cmd);
enum get_dbg_cmd_t get_dbg_cmd(struct dbg_cmd_str_t* const cmd);
//--------------------------------------- END DEBUG ----------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//--------------------------------------- BEG EXEC -----------------------------------------
static long long exec(const struct ops_t ops, regv_t* const regs);
int main(const int argc, const char* const arg[argc]);
//--------------------------------------- END EXEC -----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//--------------------------------------- BEG CONT_T ---------------------------------------

static struct ctx_t init_ctx(
	const char* const filename, 
	const char* const func
){
	struct ctx_t ctx;
	ctx.filename = filename;
	ctx.func = func;
	return ctx;
}

static void set_line(struct ctx_t* const ctx, const long line){
	ctx->line = line - 1;
}

//--------------------------------------- END CONT_T ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------  BEG FILE_T  ---------------------------------------
static bool close(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	const int err = fclose(file->stream);
	set_line(&ctx, __LINE__);
	if (err == EOF){
		rpt_fclose(&ctx, file);
		return false;
	}
	return true;
}

static bool open(
	struct file_t* const file, 
	const char* const filename, 
	const char* const mode
){
	file->name = filename;
	file->stream = fopen(filename, mode);
	file->line = 1;
	file->pos = 1;
	file->fpi = 0;
	file->fpi_pos = 0;
	if (!file->stream){
		rpt_fopen_err(file->name);
		return false;
	}
	return true;
}


static bool open_asm(struct file_t* const file, char* const buf){
	struct ctx_t ctx;
	const char* const mode = "r";
	const int buf_mode = _IOFBF;
	const bool succ = open(file, file->name, mode);
	if (!succ)
		return false;
	ctx = init_ctx(__FILE__, __func__);
	const int err = setvbuf(file->stream, buf, buf_mode, sizeof buf);
	set_line(&ctx, __LINE__);
	if (err)
		rpt_setvbuf_err(&ctx, file, buf, buf_mode, sizeof buf);
	return true;
}

static void inc_line(struct file_t* const file) {file->line++;}
static void clr_pos(struct file_t* const file) {file->pos = 0;}
static void mov_pos(struct file_t* const file, pos_t off) {file->pos += off;}
static void inc_pos(struct file_t* const file) {mov_pos(file, 1);}
static void dec_pos(struct file_t* const file) {mov_pos(file, -1);}
static bool set_fpi(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	const pos_t fpi = ftell(file->stream);
	set_line(&ctx, __LINE__);
	if (file->fpi == -1L){
		rpt_ftell_err(&ctx, file);
		return false;
	}
	file->fpi = fpi;
	file->fpi_pos = file->pos;
	return true;
}

static enum goto_nxt_ln_t goto_nxt_ln(struct file_t* const file){
	struct ctx_t ctx;
	int c;
	ctx = init_ctx(__FILE__, __func__);
	pos_t n = 0;
	const int asn = fscanf(file->stream, "%*[^\n\r]%"POS"n", &n);
	set_line(&ctx, __LINE__);
	mov_pos(file, n);
	{
		const enum chk_fscanf_t ret = chk_fscanf(&ctx, file, 0, asn);
		switch (ret){
			case chk_fscanf_ok:
				break;
			case chk_fscanf_asn:
				return goto_nxt_ln_err;
			case chk_fscanf_eof:
			case chk_fscanf_err:
				static_assert((int)goto_nxt_ln_eof == (int)chk_fscanf_eof);
				static_assert((int)goto_nxt_ln_err == (int)chk_fscanf_err);
				return (enum goto_nxt_ln_t)ret;	
			default:
				UNREACHABLE;
		}
	}
	{
		const enum go_ovr_nl_t ret = go_ovr_nl(file, &c);
		switch(ret){
			case go_ovr_nl_eof:
				return n ? goto_nxt_ln_eof : goto_nxt_ln_eol_eof;
			case go_ovr_nl_ok:
				inc_line(file);
				return goto_nxt_ln_ok;
			case go_ovr_nl_err:
				return goto_nxt_ln_err;
			case go_ovr_nl_non_nl:
				UNREACHABLE;
			default:
				UNREACHABLE;
		}
	}
}

static pos_t get_lcnt(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	while (true){
		const enum goto_nxt_ln_t ret = goto_nxt_ln(file);
		set_line(&ctx, __LINE__);
		switch (ret){
			case goto_nxt_ln_eol_eof: {
				const pos_t lcnt = file->line - 1;
				seek_beg(file);
				return lcnt;
			} case goto_nxt_ln_eof: {
				const pos_t lcnt = file->line;
				seek_beg(file);
				return lcnt;
			} case goto_nxt_ln_err:
				rpt_int_err_cont(&ctx);
				return -1;
			case goto_nxt_ln_ok:
				break;
			default:
				UNREACHABLE;
		}
	}
}

static bool seek_fpi(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	const int err = fseek(file->stream, file->fpi, SEEK_SET);
	set_line(&ctx, __LINE__);
	if (err){
		rpt_fseek_err(&ctx, file);
		return false;
	}
	file->pos = file->fpi_pos;
	return true;
}

static void seek_beg(struct file_t* const file){
	rewind(file->stream);
	file->line = 1;
	file->pos = 1;
	//fpi, fpi_pos not reset, you could still call seek_fpi
}

static enum get_t get(int* const c, struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	*c = getc(file->stream);
	set_line(&ctx, __LINE__);
	if (*c == EOF){
		if (feof(file->stream))
			return get_eof;
		rpt_getc_err(&ctx, file);
		return get_err;
	}
	inc_pos(file);
	return get_ok;
}

static bool unget(const int c, struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	const int err = ungetc(c, file->stream);
	set_line(&ctx, __LINE__);
	const bool succ = err != EOF;
	succ ? dec_pos(file) : rpt_ungetc_err(&ctx, file, c);
	return succ;
}

static int getn(
	char* const line,
	const int n,
	const struct file_t* const file
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	assert(n > 0);
	const char* const rline = fgets(line, n, file->stream);
	set_line(&ctx, __LINE__);
	if (rline)
		return getn_ok;
	if (feof(file->stream))
		return getn_eof;
	rpt_fgets_err(&ctx, file, n);
	return getn_err;
}

static char* rdln(struct file_t* const file){
	struct ctx_t ctx;
	pos_t n;
	n = 0;
	ctx = init_ctx(__FILE__, __func__);
	{
		const bool succ = seek_fpi(file);
		set_line(&ctx, __LINE__);
		if (!succ){
			rpt_int_err_cont(&ctx);
			return NULL;
		}
	}
	{
		const int num_asn_exp = 0;
		const int asn = fscanf(file->stream, "%*[^\r\n]%"POS"n", &n);
		set_line(&ctx, __LINE__);
		const enum chk_fscanf_t ret = chk_fscanf(&ctx, file, num_asn_exp, asn);
		switch(ret){
			case chk_fscanf_ok:
			case chk_fscanf_eof:
				break;
			case chk_fscanf_err:
			case chk_fscanf_asn:
				rpt_int_err_cont(&ctx);
				return NULL;
		}
	}
	{
		const bool succ = seek_fpi(file);
		set_line(&ctx, __LINE__);
		if (!succ){
			rpt_int_err_cont(&ctx);
			return NULL;
		}
	}
	assert(n >= 0);
	++n;
	char* const line = malloc((size_t)n);
	set_line(&ctx, __LINE__);
	if (!line){
		rpt_line_malloc_err(&ctx, file, n);
		return NULL;
	}
	*line = '\0';
	if (!n) //valid senario -> empty line
		return line;
	assert(n <= INT_MAX);
	const enum getn_t ret = getn(line, (int)n, file);
	set_line(&ctx, __LINE__);
	if (ret == getn_err){
		rpt_int_err_cont(&ctx);
		return NULL;
	}
	return line;
}

static bool rdln_prln(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	char* line = rdln(file);
	set_line(&ctx, __LINE__);
	if (!line){
		rpt_int_err_cont(&ctx);
		return false;
	}
	printf("%"POS"d | %s\n", file->line, line);
	free(line);
	return true;
}

static enum chk_fscanf_t chk_fscanf(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int num_asn_exp,
	const int asn
){
	if (asn == EOF){
		if (feof(file->stream))
			return chk_fscanf_eof;
		rpt_fscanf_err(ctx, file);
		return chk_fscanf_err;
	} else if (asn != num_asn_exp){
		rpt_fscanf_asn_err(ctx, file, num_asn_exp, asn);
		return chk_fscanf_asn;
	}
	return chk_fscanf_ok;
}

static enum chk_fscanf_t chk_fscanf_dpae(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int num_asn_exp,
	const int asn
){
	if (asn == EOF){
		if (feof(file->stream))
			return chk_fscanf_eof;
		rpt_fscanf_err(ctx, file);
		return chk_fscanf_err;
	} else if (asn != num_asn_exp){
		return chk_fscanf_asn;
	}
	return chk_fscanf_ok;
}

static enum go_ovr_nl_t go_ovr_chk_get(
	struct ctx_t* const ctx,
	long line,
	enum get_t ret
){
	set_line(ctx, line);
	rpt_int_err_cont_if(ctx, ret == get_err);
	static_assert((int)get_eof == go_ovr_nl_eof);
	static_assert((int)get_err == go_ovr_nl_err);
	static_assert((int)get_ok == go_ovr_nl_ok); 
	assert(ret == get_eof || ret == get_err || ret == get_ok);
	return (enum go_ovr_nl_t)ret;
}

static enum go_ovr_nl_t go_ovr_nl(struct file_t* const file, int* const o_c){
	struct ctx_t ctx;
	int c;
	ctx = init_ctx(__FILE__, __func__);
	{
		const enum get_t ret = get(&c, file);
		const enum go_ovr_nl_t ret2 = go_ovr_chk_get(&ctx, __LINE__, ret);
		if (ret2 != go_ovr_nl_ok){
			return ret2;
		}
	}
	if (c == (unsigned char)'\r'){
		{
			const enum get_t ret = get(&c, file);
			const enum go_ovr_nl_t ret2 = go_ovr_chk_get(&ctx, __LINE__, ret);
			if (ret2 != go_ovr_nl_ok){
				return ret2;
			}
		}
		if (c == (unsigned char)'\n')
			return go_ovr_nl_ok;
		const bool succ = unget(c, file);
		return prop_err(&ctx, __LINE__, succ, go_ovr_nl_ok, go_ovr_nl_err);
	}
	if (c == (unsigned char)'\n')
		return go_ovr_nl_ok;
	*o_c = c;
	return go_ovr_nl_non_nl;
}

void init_stdoutf(void){
	stdoutf.stream = stdout;
	stdoutf.name = "stdout";
	stdoutf.line = 1;
	stdoutf.pos = 0;
}

//-------------------------------------  END FILE_T  ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG USR RPT ---------------------------------------

static void rpt_usr_err_pre(void){
	fputs("USER ERROR: ", stdout);
}

static void rpt_few_args(int argc){
	rpt_usr_err_pre();
	printf("Insufficent command line arguments recieved. %d command line arguments were received. Expected at least %d\n", argc, min_num_cmd_args);
}

static void rpt_mny_args(int argc){
	rpt_usr_err_pre();
	printf("Excess command line arguments received. %d command line arguments were received. Expected at most %d\n", argc, max_num_cmd_args);
}

static void rpt_fopen_err(const char* const filename){
	rpt_usr_err_pre();
	printf("Failed to open file '%s'\n", filename);
}

static void rpt_regv_not_int(const char* const reg_str, const int reg_num){
	rpt_usr_err_pre();
	printf("register value '%s' (argument %d) for register r[%d] is not an integer\n",
		reg_str, reg_num + min_num_cmd_args, reg_num);
}

static void rpt_inv_regv(const char* reg_str, const int reg_num){
	rpt_usr_err_pre();
	printf("register value '%s' (argument %d) for register r[%d] is not a valid unsigned integer between [%"PRIregv", %"PRIregv"]\n", reg_str, min_num_cmd_args + reg_num, reg_num, MIN_REGV, MAX_REGV);
}

static void rpt_conc_div_tst_many_args(void){
	rpt_usr_err_pre();
	printf("command line argument found after the flag %s. No command line arguments are permitted after this flag\n", DIV_TST);
}

static void rpt_conc_div_tst(const char* flag){
	rpt_usr_err_pre();
	printf("Expected command line argument %d to specify whether to thoroughly test the division algorithm. Instead received '%s' which is not a valid specifier for whether to use the division algorithm\n", cmd_ln_arg_conc_div_test, flag); 
}

//-------------------------------------- END USR RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG INT RPT ---------------------------------------

static void rpt_err_pre(const char* const type, const struct file_t* const file){
	printf("%s ERROR (%s-%"POS"d-%"POS"d): ", type, file->name, file->line, file->pos);
}

static void rpt_int_err_cont(const struct ctx_t* const ctx){
	printf("\tcalled at %s/%s/%ld\n", ctx->filename, ctx->func, ctx->line);
}

static void rpt_int_err_cont_if(const struct ctx_t* const ctx, const bool b){
	if (b)
		rpt_int_err_cont(ctx);
}

static void rpt_int_err_pre(const struct file_t* const file){
	rpt_err_pre("INTERNAL", file);
}

static void rpt_int_pre(void){
	fputs("INTERNAL ERROR:", stdout);
}

static void rpt_int_err(
	const struct ctx_t* const ctx,
	const struct file_t* const file,
	const char* const descr
){
	rpt_int_err_pre(file);
	puts(descr);
	rpt_int_err_cont(ctx);
}

static void rpt_setvbuf_err(
	const struct ctx_t* const ctx,
	const struct file_t* const file,
	const char* const buf,
	const int mode,
	const size_t buf_sz
){
	rpt_int_err_pre(file);
	const char* mode_s;
	switch (mode) {
		case _IOFBF:
			mode_s = "_IOFBF";
			break;
		case _IOLBF:
			mode_s = "_IOLBF";
			break;
		case _IONBF:
			mode_s = "_IONBF";
			break;
		default:
			mode_s = "UNKOWN";
	}
	printf("Request to set the stream buffer could not be honored or an invalid value was given for mode"
		   "\n\tAttempted to set buffer to %p of size %zu with buffering mode %s."
		   "\n\tDefaulting to the default buffer\n", buf, buf_sz, mode_s);
	rpt_int_err_cont(ctx);
}

static void rpt_fscanf_err(const struct ctx_t* const ctx, const struct file_t* const file){
	rpt_int_err(ctx, file, "failed to fscanf the file. An encoding error, or a read error has occured");
}

static void rpt_fscanf_unexp_eof(
	const struct ctx_t* const ctx, 
	const struct file_t* const file
){
	rpt_int_err(ctx, file, "failed to fscanf the file. End of file encountred unexpectedly");
}

static void rpt_fscanf_asn_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int exp,
	const int act
){
	rpt_int_err_pre(file);
	printf("fscanf read too few arguments. Expected %d. Actual %d\n", exp, act);
	rpt_int_err_cont(ctx);
}	

static void rpt_fseek_err(const struct ctx_t* const ctx, const struct file_t* const file){
	rpt_int_err(ctx, file, "failed to fseek to beggining of the line");
}

static void rpt_ftell_err(const struct ctx_t* const ctx, const struct file_t* const file){
	rpt_int_err(ctx, file, "failed to ftell the file position indicator");
}

static void rpt_malloc_err(
	const struct ctx_t* const ctx, 
	const size_t n
){
	rpt_int_pre();
	printf("failed to malloc memory. Atempted to allocate %zu bytes\n", n);
	rpt_int_err_cont(ctx);
}

static void rpt_line_malloc_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const pos_t n
){
	rpt_int_err_pre(file);
	printf("failed to malloc memory for line of length. %"POS"d bytes\n", n);
	rpt_int_err_cont(ctx);
}

static void rpt_ungetc_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int c
){
	rpt_int_err_pre(file);
	printf("failed to ungetc a character. attemted to ungetc the character '%c'\n", c);
	rpt_int_err_cont(ctx);
}

static void rpt_lcnt(
	const struct file_t* const file,
	const pos_t exp
){
	rpt_int_err_pre(file);
	printf("the encounted line count %"POS"d contradicts the previously established line count %"POS"d\n", file->line, exp);
}

static void rpt_getc_err(const struct ctx_t* const ctx, const struct file_t* const file){
	rpt_int_err(ctx, file, "failed to getc a character");
}

static void rpt_fgets_err(
	const struct ctx_t* const ctx, 
	const struct file_t* const file,
	const int n
){
	rpt_int_err_pre(file);
	printf("failed to fgets a line. Attempted to get %d characters\n", n);
	rpt_int_err_cont(ctx);
}	

static void rpt_fclose(
	const struct ctx_t* const ctx, 
	const struct file_t* const file
){
	rpt_int_err_pre(file);
	printf("failed to close %s. An error was detected", file->name);
	rpt_int_err_cont(ctx);
}

//-------------------------------------- END INT RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG STX RPT ---------------------------------------

static void rpt_stx_err_pre(struct file_t* const file){
	rpt_err_pre("SYNTAX", file);	
}

static bool rdln_prln_stx_err(struct file_t* const file){
	return rdln_prln(file);
}

static bool rpt_indent(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	puts("unexpected indent");
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_long_opc(struct file_t* const file, const struct opc_str_t* const opc){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("instruction opcode exceeds maximum length %d.\n"
		   "The opcode '%s' of length %d does not exist\n", MAX_OPC_LEN, opc->s, opc->n);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_few_oprs(struct file_t* const file, const struct oprs_t* const oprs){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("instruction has too few operands.\n"
		   "No instruction has less than %d operands, however only %d were found\n"
		   , MIN_NUM_OPRS, oprs->n);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_shrt_opc(struct file_t* const file, const struct opc_str_t* const opc){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("instruction opcode lacks minumum length %d.\n"
		   "The opcode '%s' of length %d does not exist\n", MIN_OPC_LEN, opc->s, opc->n);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_not_int(struct file_t* const file, const int c){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("expected digit or newline. Read '%c'\n", c);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}	

static bool rpt_large_opr(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	puts("integer argument is too large");
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}	

static bool rpt_spc(struct file_t* const file, const pos_t n){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("expected %d white space character between tokens. Found %"POS"d\n", SPACING, n);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}	

static bool rpt_high_opr_num(struct file_t* const file){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("too many operands (>= %d). The maximum number of operands is %d\n", MAX_NUM_OPRS_TO_READ, MAX_NUM_OPRS);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_not_opc(struct file_t* const file, const struct opc_str_t* const opc){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("no such opcode '%s'\n", opc->s);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_inv_opr_num(
	struct file_t* const file, 
	const struct opc_str_t* const opc,
	const struct oprs_t* const oprs,
	const int exp
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("expected %d for instruction '%s'. got %d\n", exp, opc->s, oprs->n);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_inv_reg(
	struct file_t* const file, 
	const struct oprs_t* const oprs,
	const int opr_num
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("invalid register operand %"PRIimm" (operand %d).\n"
		   "Expected register operand in range [%d,%d]\n", 
		oprs->b[opr_num], opr_num + 1, MIN_REG, MAX_REG);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

static bool rpt_inv_imm(
	struct file_t* const file, 
	const struct oprs_t* const oprs,
	const int opr_num
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	rpt_stx_err_pre(file);
	printf("invalid immediate operand %"PRIimm" (operand %d).\n"
		   "Expected immediate operand in range [%"PRIimm",%"PRIimm"]\n", 
		oprs->b[opr_num], opr_num, MIN_IMM, MAX_IMM);
	const bool succ = rdln_prln_stx_err(file);
	return prop_bool_err(&ctx, __LINE__, succ);
}

//-------------------------------------- END STX RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG RNT RPT ---------------------------------------

static void rpt_rnt_err_pre(){
	fputs("RUNTIME ERROR: ", stdout);
}

static void rpt_op_pre(const pos_t ip){
	printf("%"POS"d | ", ip);
}

static void rpt_regs(const regv_t* const regs){
	puts("where");
	for (int i = 0; i < NUM_REG; ++i){
		rpt_regv(i, regs[i]);
	}
}

static void rpt_cyc(long long cyc){
	if (cyc >= 0)
		printf("Execution successful with %lld cycles\n", cyc);
	else
		printf("Execution failed with %lld cycles\n", cyc);
}

static void rpt_regv(const int r, const regv_t v){
	printf("\tr[%d]=%"PRIregv"\n", r, v);
}

static void rpt_reg3_op(const struct op_t op, const pos_t ip){
	rpt_op_pre(ip);
	printf("%s %d %d %d\n", opc_strs[op.t], (int)op.d, (int)op.a, (int)op.b);
}

static void rpt_reg2_op(const struct op_t op, const pos_t ip){
	rpt_op_pre(ip);
	printf("%s %d %d", opc_strs[op.t], (int)op.d, (int)op.a);
}

static void rpt_reg_imm_op(const struct op_t op, const pos_t ip){
	rpt_op_pre(ip);
	printf("%s %d %"PRIimm"\nwith\n", opc_strs[op.t], (int)op.d, op.i);
}

static void rpt_ovf(const struct op_t op, const pos_t ip){
	rpt_rnt_err_pre();
	printf("signed integer overflow. occured while excuting instruction '%s':\n", 
		opc_strs[op.t]);
	rpt_reg3_op(op, ip);
}

static void rpt_inv_jmp(const struct op_t op, const pos_t ip, const pos_t bad_ip){
	rpt_rnt_err_pre();
	printf("invalid jump to %"POS"d", bad_ip);
	rpt_reg_imm_op(op, ip);
}


static void rpt_add(const struct op_t op, const pos_t ip){rpt_ovf(op, ip);}
static void rpt_sub(const struct op_t op, const pos_t ip){rpt_ovf(op, ip);}
static void rpt_mul(const struct op_t op, const pos_t ip){rpt_ovf(op, ip);}

static void rpt_shl(const struct op_t op, const pos_t ip){
	rpt_rnt_err_pre();
	printf("signed integer overflow. occured while excuting instruction '%s':\n", 
		opc_strs[op.t]);
	rpt_reg2_op(op, ip);
}
static void rpt_shft_amt(const struct op_t op, const pos_t ip){
	rpt_rnt_err_pre();
	printf("invalid shift operand while executing instruction '%s'. Expected value in the range [0, %d)", opc_strs[op.t], REGV_WIDTH);
	rpt_reg2_op(op, ip);
}

static void rpt_jz(
	const struct op_t op, 
	const pos_t ip, 
	const pos_t bad_ip
){
	rpt_inv_jmp(op, ip, bad_ip);
}

static void rpt_jp(
	const struct op_t op, 
	const pos_t ip, 
	const pos_t bad_ip
){
	rpt_inv_jmp(op, ip, bad_ip);
}

//-------------------------------------- END RNT RPT ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------- BEG ERR PROP ---------------------------------------

static int prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ,
	const int succ_v,
	const int err_v
){
	set_line(ctx, line);
	if (succ)
		return succ_v;
	rpt_int_err_cont(ctx);
	return err_v;
}

static bool prop_bool_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ
){
	set_line(ctx, line);
	rpt_int_err_cont_if(ctx, !succ);
	return succ;
}

//------------------------------------- END ERR PROP ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------- BEG OPC STR  ---------------------------------------


static enum get_opc_t opc_prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool stx_err
){
	return (enum get_opc_t)prop_err(ctx, line, stx_err, get_opc_stx_err, get_opc_int_err);
}

static enum get_opc_t get_opc(
	struct file_t* const file,
	struct opc_str_t* const opc
){
	pos_t r1;
	pos_t r2;
	struct ctx_t ctx; 
	const int num_asn_exp = 1;
	ctx = init_ctx(__FILE__, __func__);
	r1 = 0;
	r2 = 0;
	const char* const format = " %"POS"n%"OPC_READ_BUF_LEN_STR"s%"POS"n";
	const int asn = fscanf(file->stream, format, &r1, opc->s, &r2);
	set_line(&ctx, __LINE__);
	mov_pos(file, r2);	//will add zero if fscanf fails
	{
		const enum chk_fscanf_t ret = chk_fscanf(&ctx, file, num_asn_exp, asn);
		switch (ret){
			case chk_fscanf_asn:
				return get_opc_int_err;
			case chk_fscanf_eof:
				static_assert((int)chk_fscanf_eof == get_opc_eof);
				return (enum get_opc_t)ret;
			case chk_fscanf_err:
				static_assert((int)chk_fscanf_err == get_opc_int_err);
				rpt_int_err_cont(&ctx);
				return (enum get_opc_t)ret;
			case chk_fscanf_ok:
				break;
			default:
				UNREACHABLE;
		}
	}
	if (r1){
		const bool succ = rpt_indent(file);
		return opc_prop_err(&ctx, __LINE__, succ);
	}
	static_assert(OPC_READ_BUF_LEN <= INT_MAX);
	assert(r2 - r1 <= INT_MAX);
	assert(r2 - r1 > 0);
	opc->n = (int)(r2 - r1);
	return get_opc_ok;
}

//------------------------------------- END OPC STR  ---------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG OPR -----------------------------------------

static enum get_opr_t opr_prop_err(
	struct ctx_t* const ctx,
	const long line,
	const bool succ,
	const enum get_opr_t succ_v,
	const enum get_opr_t file_v
){
	return (enum get_opr_t)prop_err(ctx, line, succ, succ_v, file_v);
}

static enum get_opr_t opr_chk_fscanf(
	struct ctx_t* const ctx, 
	const struct file_t* const file,
	const long line,
	const int num_asn_exp,
	const int asn
){
	set_line(ctx, line);
	const enum chk_fscanf_t ret = chk_fscanf(ctx, file, num_asn_exp, asn);
	switch (ret){
		case chk_fscanf_asn:
			return get_opr_int_err;
		case chk_fscanf_eof:
		case chk_fscanf_ok:
			static_assert((int)chk_fscanf_eof == get_opr_eof);
			static_assert((int)chk_fscanf_ok  == get_opr_ok);
			return (enum get_opr_t)ret;
		case chk_fscanf_err:
			rpt_int_err_cont(ctx);
			static_assert((int)chk_fscanf_err == get_opr_int_err);
			return (enum get_opr_t)ret;
		default:
			UNREACHABLE;
	}
}

static bool is_opr_chr(const int c){
	const char neg_sgn = '-';
	return c == neg_sgn || isdigit(c);
}

static enum get_opr_t get_opr(struct file_t* const file, imm_t* const opr){
	pos_t n;
	int c;
	struct ctx_t ctx;
	const int num_asn_exp_for_int = 1;
	ctx = init_ctx(__FILE__, __func__);
	n = 0;
	{
		const int asn = fscanf(file->stream, "%*[ ]%"POS"n", &n);
		const int num_asn_exp_for_space = 0;
		const enum get_opr_t err = opr_chk_fscanf(&ctx, file, __LINE__, num_asn_exp_for_space, asn);
		mov_pos(file, n);	//adds 0 if fscanf fails
		if (err != get_opr_ok)
			return err;
	}
	{
		const enum go_ovr_nl_t ret = go_ovr_nl(file, &c);
		set_line(&ctx, __LINE__);
		switch(ret){
			case go_ovr_nl_eof:
				return get_opr_eof;
			case go_ovr_nl_ok:
				return get_opr_eol;
			case go_ovr_nl_err:
				rpt_int_err_cont(&ctx);
				return get_opr_eof;
			case go_ovr_nl_non_nl:
				break;
			default:
				UNREACHABLE;
		}
	}
	if (!is_opr_chr(c)) { //is this necessry????
		const bool succ = rpt_not_int(file, c);
		return opr_prop_err(&ctx, __LINE__, succ, get_opr_stx_err, get_opr_int_err);
	} else if (!n) {	
		const bool succ = rpt_large_opr(file);
		return opr_prop_err(&ctx, __LINE__, succ, get_opr_stx_err, get_opr_int_err);
	} else if (n != SPACING){
		const bool succ = rpt_spc(file, n);
		return opr_prop_err(&ctx, __LINE__, succ, get_opr_stx_err, get_opr_int_err);
	}
	{
		const bool succ = unget(c, file);
		set_line(&ctx, __LINE__);
		if (!succ){
			rpt_int_err_cont(&ctx);
			return get_opr_int_err;
		}
	}
	n = 0;
	const int asn = fscanf(file->stream, "%"SCNimm"%"POS"n", opr, &n);
	const enum get_opr_t err = opr_chk_fscanf(&ctx, file, __LINE__, num_asn_exp_for_int, asn);
	mov_pos(file, n); //adds 0 if fscanf fails
	if (err == get_opr_eof) 
		rpt_fscanf_unexp_eof(&ctx, file);
	return err;
}

static enum get_oprs_t get_oprs(
	struct file_t* const file, 
	struct oprs_t* const oprs
){
	struct ctx_t ctx;
	int i;
	ctx = init_ctx(__FILE__, __func__);
	for (i = 0; i < MAX_NUM_OPRS_TO_READ; ++i){
		const enum get_opr_t ret = get_opr(file, oprs->b + i);
		set_line(&ctx, __LINE__);
		switch(ret) {
			case get_opr_ok:
				continue;
			case get_opr_eol:
			case get_opr_eof:
			case get_opr_stx_err:
				static_assert((int)get_opr_eol == get_oprs_eol);
				static_assert((int)get_opr_eof == get_oprs_eof);
				static_assert((int)get_opr_stx_err == get_oprs_stx_err);
				oprs->n = i;
				return (enum get_oprs_t)ret;
			case get_opr_int_err:
				oprs->n = i;
				rpt_int_err_cont(&ctx);
				return get_oprs_int_err;
			default:
				UNREACHABLE;
		}
	}
	oprs->n = i;
	static_assert(MAX_NUM_OPRS < MAX_NUM_OPRS_TO_READ);
	const bool succ = rpt_high_opr_num(file);
	return (enum get_oprs_t)prop_err(&ctx, __LINE__, succ, get_oprs_stx_err, get_oprs_int_err);
}

//---------------------------------------- END OPR -----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG DEC -----------------------------------------

static int hash(const struct opc_str_t* const opc){
	return opc->s[opc->n - 2] - opc->s[opc->n - 1];
}

static enum dec_t dec_prop_err(struct ctx_t* const ctx, const long line, const bool succ){
	return (enum dec_t)prop_err(ctx, line, succ, dec_stx_err, dec_int_err);
}

static bool is_opr_btw(imm_t opr, int lb, int ub){
	return opr >= lb && opr <= ub;
}

static bool is_reg(imm_t reg){
	return is_opr_btw(reg, MIN_REG, MAX_REG);
}

static bool is_imm(imm_t imm){
	return is_opr_btw(imm, MIN_IMM, MAX_IMM);
}

static enum dec_t dec_rpt_int_err_cont_if(
	const struct ctx_t* const ctx, 
	const enum dec_t ret
){
	rpt_int_err_cont_if(ctx, ret == dec_int_err);
	return ret;
}

static enum dec_t dec(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs,
	struct op_t* const op
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	if (opc->n > MAX_OPC_LEN){
		const bool succ = rpt_long_opc(file, opc);
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	if (opc->n < MIN_OPC_LEN){
		const bool succ = rpt_shrt_opc(file, opc); 			
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	if (oprs->n < MIN_NUM_OPRS){
		const bool succ = rpt_few_oprs(file, oprs);
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	const int h = hash(opc);
	switch(h){
		case hopc_add :
			if (strcmp(opc->s, ADD))
				goto not_opc;
			op->t = opc_add;
			goto dec_reg3_op;
        case hopc_sub :
			if (strcmp(opc->s, SUB))
				goto not_opc;
			op->t = opc_sub;
			goto dec_reg3_op;
		case hopc_mul :
			if (strcmp(opc->s, MUL))
				goto not_opc;
			op->t = opc_mul;
			goto dec_reg3_op;
		case hopc_shl :
			if (strcmp(opc->s, SHL))
				goto not_opc;
			op->t = opc_shl;
			goto dec_reg2_op;
		case hopc_shr :
			if (strcmp(opc->s, SHR))
				goto not_opc;
			op->t = opc_shr;
			goto dec_reg2_op;
		case hopc_bor :
			if (strcmp(opc->s, BOR))
				goto not_opc;
			op->t = opc_bor;
			goto dec_reg3_op;
		case hopc_band:
			if (strcmp(opc->s, BAND))
				goto not_opc;
			op->t = opc_band;
			goto dec_reg3_op;
		case hopc_li  :
			if (strcmp(opc->s, LI))
				goto not_opc;
			op->t = opc_li;
			goto dec_reg_imm_op;
		case hopc_jz  :
			if (strcmp(opc->s, JZ))
				goto not_opc;
			op->t = opc_jz;
			goto dec_reg_imm_op;
		case hopc_jp  :
			if (strcmp(opc->s, JP))
				goto not_opc;
			op->t = opc_jp;
			goto dec_reg_imm_op;
		default:
			goto not_opc;
	}
  not_opc: {
	const bool succ = rpt_not_opc(file, opc);
	return dec_prop_err(&ctx, __LINE__, succ);
  } dec_reg3_op: {
	enum dec_t ret = dec_reg_op(file, opc, oprs, op, 3);
	set_line(&ctx, __LINE__);
	return dec_rpt_int_err_cont_if(&ctx, ret);
  } dec_reg2_op: {
	enum dec_t ret = dec_reg_op(file, opc, oprs, op, 2);
	set_line(&ctx, __LINE__);
	return dec_rpt_int_err_cont_if(&ctx, ret);	 
  } dec_reg_imm_op: {
	if (!is_reg(oprs->b[0])){
		const bool succ = rpt_inv_reg(file, oprs, 0);
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	if (!is_imm(oprs->b[1])){
		const bool succ = rpt_inv_imm(file, oprs, 1);
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wconversion"
	op->d = oprs->b[0];
	op->i = oprs->b[1];
	return dec_ok;
	#pragma GCC diagnostic pop
  }
}

static enum dec_t dec_reg_op(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs, 
	struct op_t* op,
	const int num_regs
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	{
		const enum dec_t ret = chk_opr_num(file, opc, oprs, num_regs);
		set_line(&ctx, __LINE__);
		if (ret != dec_ok)
			return dec_rpt_int_err_cont_if(&ctx, ret);
	}
	{
		const enum dec_t ret = chk_reg_oprs(file, oprs);
		set_line(&ctx, __LINE__);
		if (ret != dec_ok)
			return dec_rpt_int_err_cont_if(&ctx, ret);
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wconversion"
	op->d = oprs->b[0];
	op->a = oprs->b[1];
	op->b = oprs->b[2];
	#pragma GCC diagnostic pop
	return dec_ok;
}

static enum dec_t chk_opr_num(
	struct file_t* const file,
	const struct opc_str_t* const opc, 
	const struct oprs_t* const oprs, 
	const int num_regs
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	if (oprs->n != num_regs){
		const bool succ = rpt_inv_opr_num(file, opc, oprs, num_regs);
		return dec_prop_err(&ctx, __LINE__, succ);
	}
	return dec_ok;
}

static enum dec_t chk_reg_oprs(
	struct file_t* const file,
	const struct oprs_t* const oprs
){
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	for (int i = 0; i < oprs->n; ++i)
		if (!is_reg(oprs->b[i])){
			const bool succ = rpt_inv_reg(file, oprs, i);
			return dec_prop_err(&ctx, __LINE__, succ);
		}
	return dec_ok;
}
//---------------------------------------- END DEC -----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//---------------------------------------- BEG REGV ----------------------------------------

static bool init_reg(regv_t* const reg, const char* const reg_str, const int i){
	pos_t n;
	n = 0;
	const int ret = sscanf(reg_str, "%"SCNregv"%*s%"POS"n", reg, &n);
	if (ret == EOF || ret != 1){
		rpt_regv_not_int(reg_str, i);
		return false;
	} else if (n) {
		if (isdigit((unsigned char)reg_str[n]))
			rpt_inv_regv(reg_str, i);
		else
			rpt_regv_not_int(reg_str, i);
		return false;
	} else if (*reg >= MIN_REGV || *reg <= MAX_REGV)
		return true;
	else {
		rpt_inv_regv(reg_str, i);
		return false;
	}
}

static bool init_regs(regv_t* const regs, int n, const char* const reg_strs[n]){
	for (int i = 0; i < n; ++i){
		const bool succ = init_reg(regs + i, reg_strs[i], i);
		if (!succ)
			return false;
	}
	return true;
}

//---------------------------------------- END REGV ----------------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//-------------------------------------- BEG GET OPS ---------------------------------------

static enum get_op_t get_op(struct file_t* const file, struct op_t* op){
	struct opc_str_t opc;
	struct oprs_t oprs;
	enum get_op_t ret;
	struct ctx_t ctx;
	ctx = init_ctx(__FILE__, __func__);
	clr_pos(file);
	{
		const bool succ = set_fpi(file);
		set_line(&ctx, __LINE__);
		if (!succ){
			rpt_int_err_cont(&ctx);
			return get_op_int_err;
		}
	}
	{
		const enum get_opc_t ret = get_opc(file, &opc);
		set_line(&ctx, __LINE__);
		switch (ret){
			case get_opc_eof:
			case get_opc_stx_err:
				static_assert((int)get_opc_eof == get_op_eof);
				static_assert((int)get_opc_stx_err == get_op_stx_err);
				return (enum get_op_t)ret;
			case get_opc_int_err:
				rpt_int_err_cont(&ctx);
				return get_op_int_err;
			case get_opc_ok:
				break;
			default:
				UNREACHABLE;
		}
	}
	{
		const enum get_oprs_t ret2 = get_oprs(file, &oprs);
		set_line(&ctx, __LINE__);
		switch (ret2){
			case get_oprs_int_err:
				rpt_int_err_cont(&ctx);
				return get_op_int_err;
			case get_oprs_stx_err:
				return get_op_stx_err;
			case get_oprs_eof:
				ret = get_op_eof;
				break;
			case get_oprs_eol:
				ret = get_op_ok;
				break;
			default:
				UNREACHABLE;
		}
	}
	const enum dec_t decv = dec(file, &opc, &oprs, op);
	set_line(&ctx, __LINE__);
	switch(decv){
		case dec_ok:
			break;
		case dec_int_err:
			rpt_int_err_cont(&ctx);
			return get_op_int_err;
		case dec_stx_err:
			return get_op_stx_err;
		default:
			UNREACHABLE;
	}
	return ret;
}

static enum get_ops_t get_ops(const char* filename, struct ops_t* o_ops){
	struct ctx_t ctx;
	char buf[1000]; //file needs to be closed before main ends, else UB
	struct file_t file;
	enum get_ops_t ret;
	ctx = init_ctx(__FILE__, __func__);
	file.name = filename;
	{
		const bool succ = open_asm(&file, buf);
		if (!succ)
			return EXIT_FAILURE;
	}
	const pos_t num_ops = get_lcnt(&file);
	set_line(&ctx, __LINE__);
	if (num_ops == -1){
		rpt_int_err_cont(&ctx);
		return get_ops_int_err;
	}
	assert(num_ops >= 0);
	assert(sizeof num_ops < sizeof(size_t) || (unsigned long long) num_ops <= SIZE_MAX);
	assert((size_t)num_ops <= SIZE_MAX / sizeof(struct op_t));
	struct op_t* const ops = malloc((size_t)num_ops * sizeof *ops);
	set_line(&ctx, __LINE__);
	if (!ops){
		rpt_malloc_err(&ctx, (size_t)num_ops * sizeof *ops);
		ret = get_ops_int_err;
		goto close;
	}
	for (;; inc_line(&file)){
		if (file.line > num_ops){
			rpt_lcnt(&file, num_ops);
			ret = get_ops_int_err;
			goto free_close;
		}
		const enum get_op_t op_ret = get_op(&file, ops + file.line - 1);
		set_line(&ctx, __LINE__);
		switch (op_ret){
			case get_op_eof:
				o_ops->n = (int)num_ops;
				o_ops->b = ops;
				ret = get_ops_ok;
				goto close;
			case get_op_ok:
				break;
			case get_op_int_err:
				rpt_int_err_cont(&ctx);
				ret = get_ops_int_err;
				goto free_close;
			case get_op_stx_err:
			case get_op_usr_err:
				ret = (enum get_ops_t)op_ret;
				goto free_close;
			default:
				UNREACHABLE;
		}
	}
  free_close:
  close:
	const bool succ = close(&file);
	return (enum get_ops_t)prop_err(&ctx, __LINE__, succ, ret, get_ops_int_err);
}

//-------------------------------------- END GET OPS ---------------------------------------
//------------------------------------------------------------------------------------------
//ok valid commands:
/*
	step - move to the next line
	continue - move to the next breakpoint
	disable
	enable
	print [r1 ... rn] | [rs]
	break <line number>
	run
	kill
*/

int dbg_hash(const struct dbg_cmd_str_t* const cmd){
	return cmd->s[0];
}

enum get_dbg_cmd_t get_dbg_cmd(struct dbg_cmd_str_t* const cmd){
	struct ctx_t ctx;
	int n;
	ctx = init_ctx(__FILE__, __func__);
	n = 0;
	const int exp_asn = 1;
	const int asn = fscanf(stdoutf.stream, " %"MAX_DBG_CMD_TO_READ_STR"s%n", cmd->s, &n);
	set_line(&ctx, __LINE__);
	mov_pos(&stdoutf, n);
	const enum chk_fscanf_t ret = chk_fscanf_dpae(&ctx, &stdoutf, exp_asn, asn);
	//not finished
	(void)ret;
	return 0;
	//might need to use a different function for fscanf
	//go through different cases here
}

static long long dbg(const struct ops_t ops, regv_t* const regs){
	long long cyc;
	pos_t ip;
	ip = 0;
	cyc = 0;
	/* get command here */
	while (ip != ops.n){
		const struct op_t op = ops.b[ip];
		++ip;
		++cyc;
		switch(op.t){
			case opc_add: {
				bool const err = ckd_add(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_add(op, ip);
				return -cyc; 
			} case opc_sub: {
				bool const err = ckd_sub(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_sub(op, ip);
				return -cyc;
			} case opc_mul: {
				cyc += 2;
				bool const err = ckd_mul(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_mul(op, ip);
				return -cyc;
			} case opc_shl: { 
				if (regs[op.a] < 0 || regs[op.a] >= REGV_WIDTH){
					rpt_shft_amt(op, ip);
					return -cyc;
				}
				const int_fast64_t res = (int_fast64_t)regs[op.d] << regs[op.a]; //sign extend 64 bits, before shifting
				if (res >= MIN_REGV){
					if (res <= MAX_REGV){
						regs[op.d] = (regv_t)res;
						break;
					} else if (res <= (long long)MAX_UREGV){
						//res can be represented in 32-bits
						//however the msb is 1
						//needs to be converted to the correspoding twos complement integer
						//i.e. res - REGV_MAX + REGV_MIN 
						regs[op.d] = (regv_t)(res - MAX_REGV - 1 + MIN_REGV);
						break;
					}
				}
				rpt_shl(op, ip);
				return -cyc;
			} case opc_shr:
				//TODO find better asr alternative. not guarenteed to sign extend
				if (regs[op.a] < 0 || regs[op.a] >= REGV_WIDTH){
					rpt_shft_amt(op, ip);
					return -cyc;
				}
				if (regs[op.d] < 0)
					regs[op.d] = -(imm_t)((uimm_t)regs[op.d] >> regs[op.a]);
				else 
					regs[op.d] = regs[op.d] >> regs[op.a];
				break;
			case opc_bor: 
				regs[op.d] = regs[op.d] | regs[op.a];
				break;
			case opc_band:
				regs[op.d] = regs[op.d] & regs[op.a];
				break;
			case opc_li:
				regs[op.d] = op.i;
				break;
			case opc_jz: {
				cyc += 2;
				if (regs[op.d])
					break;
				ip += op.i - 1;
				if (ip >= 0 && ip <= ops.n)
					break;
				const pos_t inv_ip = ip;
				ip -= op.i - 1;
				rpt_jz(op, ip, inv_ip);
				return -cyc;
			} case opc_jp: { 
				cyc += 2;
				if (regs[op.d] <= 0)
					break;
				ip += op.i - 1;
				if (ip >= 0 && ip <= ops.n)
					break;
				const pos_t inv_ip = ip;
				ip -= op.i - 1;
				rpt_jp(op, ip, inv_ip);
				return -cyc;
			}
		}
	}
	(void)ip;
	(void)cyc;
	(void)ops;
	(void)regs;
	return cyc;
}

static long long exec(const struct ops_t ops, regv_t* const regs){
	long long cyc;
	pos_t ip;
	ip = 0;
	cyc = 0;
	while (ip != ops.n){
		const struct op_t op = ops.b[ip];
		++ip;
		++cyc;
		switch(op.t){
			case opc_add: {
				bool const err = ckd_add(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_add(op, ip);
				return -cyc; 
			} case opc_sub: {
				bool const err = ckd_sub(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_sub(op, ip);
				return -cyc;
			} case opc_mul: {
				cyc += 2;
				bool const err = ckd_mul(regs + op.d, regs[op.a], regs[op.b]);
				if (!err)
					break;
				rpt_mul(op, ip);
				return -cyc;
			} case opc_shl: { 
				if (regs[op.a] < 0 || regs[op.a] >= REGV_WIDTH){
					rpt_shft_amt(op, ip);
					return -cyc;
				}
				const int_fast64_t res = (int_fast64_t)regs[op.d] << regs[op.a]; //sign extend 64 bits, before shifting
				if (res >= MIN_REGV){
					if (res <= MAX_REGV){
						regs[op.d] = (regv_t)res;
						break;
					} else if (res <= (long long)MAX_UREGV){
						//res can be represented in 32-bits
						//however the msb is 1
						//needs to be converted to the correspoding twos complement integer
						//i.e. res - REGV_MAX + REGV_MIN 
						regs[op.d] = (regv_t)(res - MAX_REGV - 1 + MIN_REGV);
						break;
					}
				}
				rpt_shl(op, ip);
				return -cyc;
			} case opc_shr:
				//TODO find better asr alternative. not guarenteed to sign extend
				if (regs[op.a] < 0 || regs[op.a] >= REGV_WIDTH){
					rpt_shft_amt(op, ip);
					return -cyc;
				}
				if (regs[op.d] < 0)
					regs[op.d] = -(imm_t)((uimm_t)regs[op.d] >> regs[op.a]);
				else 
					regs[op.d] = regs[op.d] >> regs[op.a];
				break;
			case opc_bor: 
				regs[op.d] = regs[op.d] | regs[op.a];
				break;
			case opc_band:
				regs[op.d] = regs[op.d] & regs[op.a];
				break;
			case opc_li:
				regs[op.d] = op.i;
				break;
			case opc_jz: {
				cyc += 2;
				if (regs[op.d])
					break;
				ip += op.i - 1;
				if (ip >= 0 && ip <= ops.n)
					break;
				const pos_t inv_ip = ip;
				ip -= op.i - 1;
				rpt_jz(op, ip, inv_ip);
				return -cyc;
			} case opc_jp: { 
				cyc += 2;
				if (regs[op.d] <= 0)
					break;
				ip += op.i - 1;
				if (ip >= 0 && ip <= ops.n)
					break;
				const pos_t inv_ip = ip;
				ip -= op.i - 1;
				rpt_jp(op, ip, inv_ip);
				return -cyc;
			}
		}
	}
	return cyc;
}

int is_div_tst(const char* const flag){
	{
		const bool conc_div_test = !strcmp(flag, DIV_TST);
		if (conc_div_test)
			return true;
	}
	const bool no_conc_div_test = !strcmp(flag, NO_DIV_TST);
	return no_conc_div_test ? false : -1;
}

int main(const int argc, const char* const argv[argc]){
	struct ctx_t ctx;
	struct ops_t ops;
	regv_t regs[NUM_REG] = {};
	bool succ;
	init_stdoutf();
	if (argc < min_num_cmd_args){
		rpt_few_args(argc);
		return EXIT_FAILURE;
	} else if (argc > max_num_cmd_args){
		rpt_mny_args(argc);
		return EXIT_FAILURE;
	}
	
	const int div_tst = is_div_tst(argv[cmd_ln_arg_conc_div_test]);
	if (div_tst == -1){
		rpt_conc_div_tst(argv[cmd_ln_arg_conc_div_test]);
		return EXIT_FAILURE;
	}

	if (!div_tst){
		const int num_to_init = argc - min_num_cmd_args;
		const char* const* const reg_strs = argv + min_num_cmd_args;
		succ = init_regs(regs, num_to_init, reg_strs);
		if (!succ)
			return EXIT_FAILURE;
	} else if (argc > min_num_cmd_args) {
		rpt_conc_div_tst_many_args();
		return EXIT_FAILURE;
	}
	
	const char* const filename = argv[cmd_ln_arg_file_name];
	ctx = init_ctx(__FILE__, __func__);
	const enum get_ops_t ret = get_ops(filename, &ops);
	set_line(&ctx, __LINE__);
	if (ret != get_ops_ok){
		rpt_int_err_cont_if(&ctx, ret == get_ops_int_err);
		free(ops.b);
		return EXIT_FAILURE;
	}
	const long long cyc = (/* some condition*/ 0) ? dbg(ops, regs) : exec(ops, regs);
	rpt_regs(regs);
	rpt_cyc(cyc);
	free(ops.b);
	return EXIT_SUCCESS;
}