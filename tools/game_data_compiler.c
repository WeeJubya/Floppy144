/*
 * Floppy//144 Game Data Compiler
 *
 * Development-only strict JSON -> generated C/X-macro + site JSON compiler.
 * The parser and source JSON do not need to ship with Floppy144.exe.
 *
 * C99, standard library only.
 */
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum JsonType { J_NULL=0, J_BOOL, J_NUMBER, J_STRING, J_ARRAY, J_OBJECT } JsonType;
typedef struct JsonValue JsonValue;
typedef struct JsonMember { char *key; JsonValue *value; } JsonMember;
struct JsonValue {
    JsonType type;
    union {
        bool boolean;
        double number;
        char *string;
        struct { JsonValue **items; size_t count, cap; } array;
        struct { JsonMember *items; size_t count, cap; } object;
    } as;
};
typedef struct Parser { const char *s; size_t n,p; int line,col; const char *error; } Parser;

static void die(const char *m){ fprintf(stderr,"ERROR: %s\n",m); exit(1); }
static void *xmalloc(size_t n){ void *p=malloc(n?n:1); if(!p) die("out of memory"); return p; }
static void *xrealloc(void *p,size_t n){ void *q=realloc(p,n?n:1); if(!q) die("out of memory"); return q; }
static char *xstrdup(const char *s){ size_t n=strlen(s)+1; char *p=(char*)xmalloc(n); memcpy(p,s,n); return p; }
static JsonValue *jnew(JsonType t){ JsonValue *v=(JsonValue*)calloc(1,sizeof(*v)); if(!v) die("out of memory"); v->type=t; return v; }
static void skip_ws(Parser *p){ while(p->p<p->n){ unsigned char c=(unsigned char)p->s[p->p]; if(!isspace(c)) break; p->p++; if(c=='\n'){p->line++;p->col=1;} else p->col++; } }
static int peek(Parser *p){ skip_ws(p); return p->p<p->n?(unsigned char)p->s[p->p]:-1; }
static int take(Parser *p){ if(p->p>=p->n)return -1; {unsigned char c=(unsigned char)p->s[p->p++]; if(c=='\n'){p->line++;p->col=1;}else p->col++; return c;} }
static void fail(Parser *p,const char *m){ if(!p->error)p->error=m; }
static void append_byte(char **buf,size_t *len,size_t *cap,unsigned char c){ if(*len+2>*cap){*cap=*cap?*cap*2:32;*buf=(char*)xrealloc(*buf,*cap);} (*buf)[(*len)++]=(char)c; (*buf)[*len]='\0'; }
static void append_utf8(char **b,size_t *l,size_t *c,unsigned u){ if(u<=0x7f) append_byte(b,l,c,(unsigned char)u); else if(u<=0x7ff){append_byte(b,l,c,(unsigned char)(0xc0|(u>>6)));append_byte(b,l,c,(unsigned char)(0x80|(u&63)));} else {append_byte(b,l,c,(unsigned char)(0xe0|(u>>12)));append_byte(b,l,c,(unsigned char)(0x80|((u>>6)&63)));append_byte(b,l,c,(unsigned char)(0x80|(u&63)));} }
static int hexv(int c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }
static char *parse_string_raw(Parser *p){ char *b=NULL; size_t l=0,c=0; if(take(p)!='\"'){fail(p,"expected string");return NULL;} while(p->p<p->n){ int ch=take(p); if(ch=='\"') return b?b:xstrdup(""); if(ch=='\\'){ int e=take(p); switch(e){case '\"':append_byte(&b,&l,&c,'\"');break;case '\\':append_byte(&b,&l,&c,'\\');break;case '/':append_byte(&b,&l,&c,'/');break;case 'b':append_byte(&b,&l,&c,'\b');break;case 'f':append_byte(&b,&l,&c,'\f');break;case 'n':append_byte(&b,&l,&c,'\n');break;case 'r':append_byte(&b,&l,&c,'\r');break;case 't':append_byte(&b,&l,&c,'\t');break;case 'u':{unsigned u=0;int i;for(i=0;i<4;i++){int h=hexv(take(p));if(h<0){fail(p,"bad unicode escape");free(b);return NULL;}u=(u<<4)|(unsigned)h;}append_utf8(&b,&l,&c,u);break;}default:fail(p,"bad string escape");free(b);return NULL;} } else { if(ch<0x20){fail(p,"control character in string");free(b);return NULL;} append_byte(&b,&l,&c,(unsigned char)ch); } } fail(p,"unterminated string");free(b);return NULL; }
static JsonValue *parse_value(Parser *p);
static void arr_add(JsonValue *a,JsonValue *v){ if(a->as.array.count==a->as.array.cap){a->as.array.cap=a->as.array.cap?a->as.array.cap*2:8;a->as.array.items=(JsonValue**)xrealloc(a->as.array.items,a->as.array.cap*sizeof(*a->as.array.items));}a->as.array.items[a->as.array.count++]=v; }
static void obj_add(JsonValue *o,char *k,JsonValue *v){ if(o->as.object.count==o->as.object.cap){o->as.object.cap=o->as.object.cap?o->as.object.cap*2:8;o->as.object.items=(JsonMember*)xrealloc(o->as.object.items,o->as.object.cap*sizeof(*o->as.object.items));}o->as.object.items[o->as.object.count].key=k;o->as.object.items[o->as.object.count].value=v;o->as.object.count++; }
static JsonValue *parse_array(Parser *p){ JsonValue *a=jnew(J_ARRAY); take(p); skip_ws(p); if(peek(p)==']'){take(p);return a;} for(;;){ JsonValue *v=parse_value(p); if(!v)return a; arr_add(a,v); skip_ws(p); if(peek(p)==','){take(p);continue;} if(peek(p)==']'){take(p);return a;} fail(p,"expected ',' or ']' in array");return a; } }
static JsonValue *parse_object(Parser *p){ JsonValue *o=jnew(J_OBJECT); take(p); skip_ws(p); if(peek(p)=='}'){take(p);return o;} for(;;){ char *k; JsonValue *v; if(peek(p)!='\"'){fail(p,"expected object key");return o;} k=parse_string_raw(p); skip_ws(p); if(take(p)!=':'){fail(p,"expected ':'");free(k);return o;} v=parse_value(p); if(!v){free(k);return o;} obj_add(o,k,v); skip_ws(p); if(peek(p)==','){take(p);continue;} if(peek(p)=='}'){take(p);return o;} fail(p,"expected ',' or '}' in object");return o; } }
static bool match(Parser *p,const char *w){ size_t n=strlen(w); skip_ws(p); if(p->p+n<=p->n && memcmp(p->s+p->p,w,n)==0){p->p+=n;p->col+=(int)n;return true;}return false; }
static JsonValue *parse_value(Parser *p){ int c=peek(p); if(c=='{')return parse_object(p); if(c=='[')return parse_array(p); if(c=='\"'){JsonValue *v=jnew(J_STRING);v->as.string=parse_string_raw(p);return v;} if(c=='t'&&match(p,"true")){JsonValue*v=jnew(J_BOOL);v->as.boolean=true;return v;} if(c=='f'&&match(p,"false")){JsonValue*v=jnew(J_BOOL);v->as.boolean=false;return v;} if(c=='n'&&match(p,"null"))return jnew(J_NULL); if(c=='-'||isdigit(c)){char *end; JsonValue*v=jnew(J_NUMBER); errno=0; v->as.number=strtod(p->s+p->p,&end); if(errno||end==p->s+p->p){fail(p,"bad number");return v;} p->col+=(int)(end-(p->s+p->p));p->p=(size_t)(end-p->s);return v;} fail(p,"unexpected JSON value");return NULL; }
static void jfree(JsonValue *v){size_t i;if(!v)return; if(v->type==J_STRING)free(v->as.string); else if(v->type==J_ARRAY){for(i=0;i<v->as.array.count;i++)jfree(v->as.array.items[i]);free(v->as.array.items);} else if(v->type==J_OBJECT){for(i=0;i<v->as.object.count;i++){free(v->as.object.items[i].key);jfree(v->as.object.items[i].value);}free(v->as.object.items);}free(v);}
static JsonValue *get(JsonValue *o,const char *k){size_t i;if(!o||o->type!=J_OBJECT)return NULL;for(i=0;i<o->as.object.count;i++)if(strcmp(o->as.object.items[i].key,k)==0)return o->as.object.items[i].value;return NULL;}
static const char *strv(JsonValue *o,const char *k){JsonValue*v=get(o,k);return(v&&v->type==J_STRING)?v->as.string:NULL;}
static long intv(JsonValue *o,const char *k,long d){JsonValue*v=get(o,k);return(v&&v->type==J_NUMBER)?(long)v->as.number:d;}
static size_t count(JsonValue *a){return(a&&a->type==J_ARRAY)?a->as.array.count:0;}
static JsonValue *at(JsonValue *a,size_t i){return(a&&a->type==J_ARRAY&&i<a->as.array.count)?a->as.array.items[i]:NULL;}
static bool boolv(JsonValue *pObject,const char *pszKey,bool bDefault);
static void cstr(FILE *f,const char *s){const unsigned char*p=(const unsigned char*)(s?s:"");fputc('"',f);while(*p){unsigned char c=*p++;if(c=='\\'||c=='\"'){fputc('\\',f);fputc(c,f);}else if(c=='\n')fputs("\\n",f);else if(c=='\r')fputs("\\r",f);else if(c=='\t')fputs("\\t",f);else if(c<32)fprintf(f,"\\x%02X",c);else fputc(c,f);}fputc('"',f);}
static void sym(FILE*f,const char*id){for(;*id;id++){unsigned char c=(unsigned char)*id;if(isalnum(c))fputc((int)toupper(c),f);else if(c=='-'||c=='/'||c==' '){}else fputc('_',f);}}
static const char *domain_enum(const char*d){if(!d)return"FLOPPY144_COLLECTION_DOMAIN_DR";if(strstr(d,"Human"))return"FLOPPY144_COLLECTION_DOMAIN_HR";if(strstr(d,"Facilities"))return"FLOPPY144_COLLECTION_DOMAIN_FM";if(strstr(d,"Operational"))return"FLOPPY144_COLLECTION_DOMAIN_OS";if(strstr(d,"Technology"))return"FLOPPY144_COLLECTION_DOMAIN_TS";return"FLOPPY144_COLLECTION_DOMAIN_DR";}
static void ensure_dir(const char *p){(void)p;/* caller creates output tree; intentionally portable */}
static FILE *openout(const char*dir,const char*name){char p[1024];snprintf(p,sizeof(p),"%s/%s",dir,name);{FILE*f=fopen(p,"wb");if(!f){fprintf(stderr,"ERROR: cannot write %s\n",p);exit(1);}return f;}}
/* compact recursive writer kept separate to avoid allocating wrapper nodes */
static void write_json2(FILE*f,JsonValue*v){size_t i;switch(v->type){case J_NULL:fputs("null",f);break;case J_BOOL:fputs(v->as.boolean?"true":"false",f);break;case J_NUMBER:fprintf(f,"%.15g",v->as.number);break;case J_STRING:{const unsigned char*p=(const unsigned char*)v->as.string;fputc('"',f);while(*p){unsigned char c=*p++;if(c=='\"'||c=='\\'){fputc('\\',f);fputc(c,f);}else if(c=='\n')fputs("\\n",f);else if(c=='\r')fputs("\\r",f);else if(c=='\t')fputs("\\t",f);else if(c<32)fprintf(f,"\\u%04x",c);else fputc(c,f);}fputc('"',f);break;}case J_ARRAY:fputc('[',f);for(i=0;i<v->as.array.count;i++){if(i)fputc(',',f);write_json2(f,v->as.array.items[i]);}fputc(']',f);break;case J_OBJECT:fputc('{',f);for(i=0;i<v->as.object.count;i++){if(i)fputc(',',f);cstr(f,v->as.object.items[i].key);fputc(':',f);write_json2(f,v->as.object.items[i].value);}fputc('}',f);break;}}
static void emit_simple_def(JsonValue*root,const char*key,const char*macro,const char*outfile,const char*outdir){size_t i;JsonValue*a=get(root,key);FILE*f=openout(outdir,outfile);fprintf(f,"/* Generated from floppy144_game_data.json. Do not edit. */\n");for(i=0;i<count(a);i++){const char*id=strv(at(a,i),"id");if(!id)die("entry without id");fprintf(f,"%s(",macro);sym(f,id);fprintf(f,", ");{char compact[32];size_t j=0;const char*p=id;while(*p&&j+1<sizeof(compact)){if(*p!='-')compact[j++]=*p;p++;}compact[j]='\0';cstr(f,compact);}fprintf(f,")\n");}fclose(f);}
static size_t authored_count_for(JsonValue*docs,const char*cid){size_t i,n=0;for(i=0;i<count(docs);i++)if(strv(at(docs,i),"collection_id")&&strcmp(strv(at(docs,i),"collection_id"),cid)==0)n++;return n;}
static unsigned long stable_hash(const char *psz)
{
    unsigned long h = 2166136261UL;
    const unsigned char *p = (const unsigned char *)(psz ? psz : "");
    while(*p) { h ^= (unsigned long)*p++; h *= 16777619UL; }
    return h;
}
static long collection_record_count(JsonValue *root,const char *cid)
{
    JsonValue *a=get(root,"collections"); size_t i;
    for(i=0;i<count(a);++i){JsonValue*c=at(a,i),*b=get(c,"content_budget");if(strv(c,"id")&&strcmp(strv(c,"id"),cid)==0){long n=b?intv(b,"records",0):0;long ac=(long)authored_count_for(get(root,"documents"),cid);return n<ac?ac:n;}}
    return 0;
}
static void emit_collections(JsonValue*root,const char*outdir)
{
    JsonValue*a=get(root,"collections"),*docs=get(root,"documents");size_t i;FILE*f=openout(outdir,"floppy144_collections.generated.def");
    fputs("/* Generated from floppy144_game_data.json. Do not edit. */\n",f);
    for(i=0;i<count(a);i++){
        JsonValue*c=at(a,i),*b=get(c,"content_budget");const char*id=strv(c,"id"),*name=strv(c,"name"),*domain=strv(c,"domain");
        long size_kb=intv(c,"size_kb",0),records=0;int required=boolv(c,"required_for_completion",false)?1:0;size_t ac=authored_count_for(docs,id);
        if(b&&b->type==J_OBJECT)records=intv(b,"records",0);if(records<(long)ac)records=(long)ac;
        fprintf(f,"FLOPPY144_COLLECTION(\n    ");sym(f,id);fprintf(f,", ");cstr(f,id);fprintf(f,", ");cstr(f,name);
        fprintf(f,",\n    %s, %ldU, %s,\n    ",domain_enum(domain),size_kb,required?"true":"false");
        cstr(f,"DATA-DRIVEN COLLECTION GENERATED FROM FLOPPY144_GAME_DATA.JSON.");fprintf(f,", NULL, %ldU, ",records);cstr(f,name);fprintf(f,", ");
        {char pref[32];snprintf(pref,sizeof(pref),"%s-RS",id);cstr(f,pref);}
        fprintf(f,",\n    floppy144_generated_generic_subjects, 12U, false, %luU, 37U, %luU\n)\n\n",1000UL+(unsigned long)i*100UL,11UL+(unsigned long)i*17UL);
    }
    fclose(f);
}
static void emit_documents(JsonValue*root,const char*outdir)
{
    JsonValue*collections=get(root,"collections"),*docs=get(root,"documents");size_t ci,di;FILE*f=openout(outdir,"floppy144_documents.generated.inc");
    fputs("/* Generated from floppy144_game_data.json. Do not edit. */\n",f);
    for(ci=0;ci<count(collections);++ci){
        JsonValue*c=at(collections,ci);const char*cid=strv(c,"id");long records=collection_record_count(root,cid);unsigned char *used;
        if(records<=0) continue;
        used=(unsigned char*)calloc((size_t)records,1U);if(!used)die("out of memory assigning document slots");
        for(di=0;di<count(docs);++di){
            JsonValue*d=at(docs,di);const char*dcid=strv(d,"collection_id"),*rid=strv(d,"id"),*title=strv(d,"title"),*body=strv(d,"body"),*tid=strv(d,"trigger_id");long idx;unsigned long h;
            if(!dcid||strcmp(dcid,cid)!=0)continue;
            h=stable_hash(cid)^(stable_hash(rid)*16777619UL);idx=(long)(h%(unsigned long)records);
            while(used[idx])idx=(idx+1L)%records;used[idx]=1U;
            fprintf(f,"    { FLOPPY144_COLLECTION_");sym(f,cid);fprintf(f,", %ldU, ",idx);cstr(f,rid);fprintf(f,", ");cstr(f,title);
            if(title && strcmp(title,"Suppression Control Panel Service Note")==0) fputs(", FLOPPY144_DOCUMENT_VIEW_FM13_SUPPRESSION_SERVICE, ",f);
            else if(title && strcmp(title,"Temporary Desk Reallocation Notice")==0) fputs(", FLOPPY144_DOCUMENT_VIEW_HR01_DESK_REALLOCATION, ",f);
            else if(title && strcmp(title,"Disk Recovery Index")==0) fputs(", FLOPPY144_DOCUMENT_VIEW_DR01_DISK_RECOVERY_INDEX, ",f);
            else fputs(", FLOPPY144_DOCUMENT_VIEW_GENERIC, ",f);
            if(tid){fprintf(f,"FLOPPY144_TRIGGER_");sym(f,tid);}else fputs("FLOPPY144_TRIGGER_COUNT",f);
            fprintf(f,", NULL, 0U, ");cstr(f,body);fprintf(f," },\n");
        }
        free(used);
    }
    fclose(f);
}
static void emit_physical(JsonValue*root,const char*outdir){JsonValue*a=get(root,"physical_items");size_t i;FILE*f=openout(outdir,"floppy144_physical_items.generated.inc");fputs("/* id, name, room, parent, drawing */\n",f);for(i=0;i<count(a);i++){JsonValue*p=at(a,i);fprintf(f,"FLOPPY144_PHYSICAL_ITEM(%lu, ",(unsigned long)i);cstr(f,strv(p,"id"));fputs(", ",f);cstr(f,strv(p,"name"));fputs(", ",f);cstr(f,strv(p,"room_id"));fputs(", ",f);cstr(f,strv(p,"parent_id"));fputs(", ",f);cstr(f,strv(p,"drawing_definition_id"));fputs(")\n",f);}fclose(f);}
static void emit_ambient(JsonValue*root,const char*outdir){JsonValue*a=get(root,"ambient_interactions");size_t i;FILE*f=openout(outdir,"floppy144_ambient.generated.inc");fputs("/* Generated ambient text. */\n",f);for(i=0;i<count(a);i++){JsonValue*x=at(a,i);fprintf(f,"FLOPPY144_AMBIENT(");cstr(f,strv(x,"id"));fputs(", ",f);cstr(f,strv(x,"target_id"));fputs(", ",f);cstr(f,strv(x,"title"));fputs(", ",f);cstr(f,strv(x,"text"));fputs(")\n",f);}fclose(f);}
static void emit_runtime_ledger(JsonValue*root,const char*outdir){JsonValue *tr=get(root,"triggers"),*in=get(root,"interactions"),*ev=get(root,"evidence");size_t i,j;FILE*f=openout(outdir,"floppy144_runtime_ledger.generated.inc");fputs("/* Machine-readable generated ledger metadata. */\n",f);for(i=0;i<count(tr);i++){JsonValue*t=at(tr,i),*cs=get(t,"conditions"),*fx=get(t,"effects");fprintf(f,"FLOPPY144_TRIGGER_META(");cstr(f,strv(t,"id"));fputs(", ",f);cstr(f,strv(t,"collection_id"));fputs(", ",f);cstr(f,strv(t,"trigger_document"));fprintf(f,", %luU, %luU)\n",(unsigned long)count(cs),(unsigned long)count(fx));for(j=0;j<count(cs);j++){JsonValue*c=at(cs,j);fputs("FLOPPY144_TRIGGER_CONDITION(",f);cstr(f,strv(t,"id"));fputs(", ",f);cstr(f,strv(c,"kind"));fputs(", ",f);cstr(f,strv(c,"target"));fputs(")\n",f);}for(j=0;j<count(fx);j++){JsonValue*x=at(fx,j);fputs("FLOPPY144_TRIGGER_EFFECT(",f);cstr(f,strv(t,"id"));fputs(", ",f);cstr(f,strv(x,"op"));fputs(", ",f);cstr(f,strv(x,"target"));fputs(")\n",f);}}
for(i=0;i<count(in);i++){JsonValue*x=at(in,i);fputs("FLOPPY144_INTERACTION_META(",f);cstr(f,strv(x,"id"));fputs(", ",f);cstr(f,strv(x,"physical_source"));fputs(", ",f);cstr(f,strv(x,"player_action"));fputs(")\n",f);}for(i=0;i<count(ev);i++){JsonValue*x=at(ev,i),*sy=get(x,"synthesis"),*req=sy?get(sy,"required_interaction_ids"):NULL;fputs("FLOPPY144_EVIDENCE_META(",f);cstr(f,strv(x,"id"));fputs(", ",f);cstr(f,strv(x,"statement"));fprintf(f,", %luU)\n",(unsigned long)count(req));}fclose(f);}
#include "game_data_emit_runtime.inc"
static char *read_file(const char *path,size_t *n){FILE*f=fopen(path,"rb");char*b;long z;if(!f){fprintf(stderr,"ERROR: cannot open %s\n",path);exit(1);}fseek(f,0,SEEK_END);z=ftell(f);fseek(f,0,SEEK_SET);if(z<0)die("ftell failed");b=(char*)xmalloc((size_t)z+1);if(fread(b,1,(size_t)z,f)!=(size_t)z)die("read failed");fclose(f);b[z]='\0';*n=(size_t)z;return b;}
int main(int argc,char**argv)
{
    char*text;size_t n;Parser p;JsonValue*root,*site,*collections,*furniture;FILE*f;size_t i;unsigned long total_kb=0UL,required_kb=0UL;
    if(argc!=3){fprintf(stderr,"usage: %s floppy144_game_data.json output_directory\n",argv[0]);return 2;}
    ensure_dir(argv[2]);text=read_file(argv[1],&n);memset(&p,0,sizeof(p));p.s=text;p.n=n;p.line=1;p.col=1;root=parse_value(&p);skip_ws(&p);
    if(p.error||!root||root->type!=J_OBJECT||p.p!=p.n){fprintf(stderr,"ERROR: JSON parse failed line %d column %d: %s\n",p.line,p.col,p.error?p.error:"trailing input");return 1;}
    if(strcmp(strv(root,"game_id")?strv(root,"game_id"):"","FLOPPY144")!=0)die("wrong game_id");
    if(count(get(root,"collections"))!=35||count(get(root,"triggers"))!=50||count(get(root,"interactions"))!=40||count(get(root,"evidence"))!=23||count(get(root,"physical_items"))!=161)die("stable ledger counts do not match Floppy//144 contract");
    collections=get(root,"collections");
    for(i=0;i<count(collections);++i){JsonValue*c=at(collections,i);long kb=intv(c,"size_kb",0);if(kb<=0)die("every collection must define positive size_kb");total_kb+=(unsigned long)kb;if(boolv(c,"required_for_completion",false))required_kb+=(unsigned long)kb;}
    /* Stage 3C balance contract: the authored required route must fit, while the complete archive must not. */
    if(required_kb>1440UL)die("required collection total exceeds 1440 KB recovery capacity");
    if(total_kb<=1440UL)die("complete archive must exceed 1440 KB recovery capacity");
    furniture=get(root,"furniture");
    for(i=0;i<count(furniture);++i){JsonValue*x=at(furniture,i);if(strv(x,"variant")&&strcmp(strv(x,"variant"),"SECURE_CABINET")==0){long d=intv(x,"code_digits",0);if(d!=6&&d!=8)die("secure cabinet code_digits must be 6 or 8");}}
    emit_collections(root,argv[2]);emit_documents(root,argv[2]);emit_simple_def(root,"triggers","FLOPPY144_TRIGGER","floppy144_triggers.generated.def",argv[2]);emit_simple_def(root,"interactions","FLOPPY144_INTERACTION","floppy144_interactions.generated.def",argv[2]);emit_simple_def(root,"evidence","FLOPPY144_EVIDENCE","floppy144_evidence.generated.def",argv[2]);emit_physical(root,argv[2]);emit_ambient(root,argv[2]);emit_runtime_ledger(root,argv[2]);emit_flat_runtime(root,argv[2]);
    site=get(root,"site_layout_source");if(!site)die("site_layout_source missing");f=openout(argv[2],"site_layout.generated.jsonc");fputs("/* Generated from floppy144_game_data.json by game_data_compiler. */\n",f);write_json2(f,site);fputc('\n',f);fclose(f);
    printf("Floppy//144 game data compiled: 35 collections, 157 documents, 50 triggers, 40 interactions, 23 evidence, 161 physical items; %lu/%lu KB.\n",required_kb,total_kb);
    jfree(root);free(text);return 0;
}
