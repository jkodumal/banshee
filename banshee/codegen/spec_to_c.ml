(*
 * Copyright (c) 2000-2004
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *)

open Cgen
open Engspec

let cfst ((a,b,c) : conid) = a
      
let foldr f =
  let rec localfun y l =
    match l with [] -> y 
    |   x::xs -> f(x, (localfun y xs))
  in localfun

let foldl f = 
  let rec localfun y l =
    match l with [] -> y
    |  x::xs -> localfun (f(x,y)) xs 
  in localfun 

module type ENV =
    sig
      type map
      exception Duplicate of string 
      val insert_exprid : exprid * sort_gen * map -> map
      val lookup_exprid : exprid -> map -> sort_gen option
      val insert_conid : conid * map -> map
      val lookup_conid : conid -> map -> bool
      val empty_env : map
      val plus : map * map -> map
      val exprids_with_sorts : map -> (exprid * sort_gen) list
    end

module Env : ENV = 
  struct
    type res = 
	E of exprid * sort_gen 
      |	C of conid
    type map = res list
    exception Duplicate of string 
    let error e = e ^ " is multiply defined" 
    let lookup_exprid e m = 
      try 
	(match (List.find (function E(e',_) -> e = e' | _ -> false) m) with 
	| (E(_,s)) -> Some s
	| _ -> None ) with
      |	_ -> None
    let insert_exprid (e,s,m) = 
      match (lookup_exprid e m) with
      | None -> E(e,s) :: m
      |	Some _ -> raise (Duplicate (error e))
    let lookup_conid c m = 
      try 
	(match (List.find (function C(c') -> (cfst c) = (cfst c') | _ -> false) m) with
	| _ -> true ) with
      |	_ -> false
    let insert_conid (c,m) = 
      if (lookup_conid c m) then raise (Duplicate (error (cfst c))) else C c :: m
    let empty_env = []
    let plus (m1,m2) = 
      foldl (function (E(e,s),m) -> insert_exprid(e,s,m) 
	  | (C c,m) -> insert_conid(c,m)) m1 m2
    let exprids_with_sorts env =  foldl(function (E (e,s), a) -> (e,s) :: a
      | (C _, a) -> a) [] env
  end

let gen_opaque_type exprid header = header#add_tdecl (opaque exprid)

let gen_dataspec env dataspec header source = 
  let conids body = List.map (function (a,b) -> a) body in 
  let env' = 
    foldl (function ((e,s,body),env) -> 
      let env' = Env.insert_exprid(e,s,env) in
      foldl (function (c,env) -> Env.insert_conid(c,env)) env' (conids body))
      Env.empty_env dataspec in
  (*let env'' = Env.plus (env,env') in*)
  let _ = 
    List.map (function (e,s,body) -> 
      (gen_opaque_type e header; 
       s#gen_sort_ops source header e;
       s#gen_con_ops source header (e,body))) dataspec
(*
       match body with 
       | h :: t -> s#gen_con_ops source header (e,body)
       | [] -> () ) ) dataspec
*)
  in
  env'
  
let rec gen_dataspecs env dataspecs header source = 
  match dataspecs with
  | [] -> Env.empty_env
  | h :: t -> 
      let env' = gen_dataspec env h header source in
      let env'' = gen_dataspecs (Env.plus (env,env')) t header source in
      Env.plus (env',env'')

let gen_preamble env sigid header source  = 
  let hdr_name = String.uppercase sigid ^ "_H" in
  let hdr_ifndef = ifndef hdr_name and hdr_def = define hdr_name in
  let start_cmnt = block_comment "DO NOT edit this file" in
  let inc1 = include_header true "list.h" in
  let inc2 = include_header true "regions.h" in
  let inc3 = include_header true "banshee.h" in
  let inc4 = include_header false "assert.h" in
  let inc5 = include_header false "stdio.h" in
  let inc6 = include_header true  "bool.h" in
  let inc7 = include_header true  "ufind.h" in
  let inc8 = include_header false "string.h" in
  let inc9 = include_header true  "linkage.h" in
  let inc10 = include_header true "hash.h" in
  let inc11 = include_header true "banshee_region_persist_kinds.h" in
  let flag =
    var (no_qual Int) "flag_hash_cons" (Some Extern) ; in 
  header#add_includes [start_cmnt;hdr_ifndef;hdr_def;inc1;inc5;inc6;inc9;inc10;flag];
  source#add_includes [start_cmnt;inc1;inc2;inc3;inc4;inc5;inc6;inc7;inc8;inc10;inc11];
  header#add_macro (macro "EXTERN_C_BEGIN")

let gen_postamble env strid header source (sorts : (exprid*sort_gen) list) = 
  let return = void in
  let return2 = (no_qual (Ident "hash_table *")) in
  let init = strid ^ "_init" in
  let reset = strid ^ "_reset" in
  let stats = strid ^ "_stats" in
  let graph = strid ^ "_print_graph" in
  let serialize = strid ^ "_serialize" in
  let deserialize = strid ^ "_deserialize" in
  let serialize_rgn = strid ^ "_region_serialize" in
  let deserialize_rgn = strid ^ "_region_deserialize" in
  let formals = [] in
  let cmnt = block_comment "Init/reset engine, print constraint graphs, serialize/deserialize constraint graphs" in
  let init_engine = Expr "engine_init();" in
  let reset_engine = Expr "engine_reset();" in
  let stats_engine = Expr "engine_stats(arg1);" in
  let print_graph = Expr "print_constraint_graphs(arg1);" in
  let serialize_engine = Expr "return;" in
  let deserialize_engine = Expr "return NULL;" in
  let sort_inits = 
    foldr (function ((e,s),acc) -> (s#init e) @ acc) [] sorts in 
  let sort_resets = foldr 
      (function ((e,s),acc) -> (s#reset e) @ acc) [] sorts in 
  let init_body = init_engine :: sort_inits in
  let reset_body = reset_engine :: sort_resets in
  let stats_body = [stats_engine] in
  let graph_body = [print_graph] in
  let serialize_body = [serialize_engine] in
  let deserialize_body = [deserialize_engine] in
  let init_fun = (return,init,formals,init_body,[]) in
  let reset_fun = (return,reset,formals,reset_body,[Deletes]) in
  let stats_fun = (return,stats,[(no_qual (Ident "FILE *"),"arg1")],
				 stats_body,[]) in
  let graph_fun = (return,graph,[(no_qual (Ident "FILE *"),"arg1")] ,
		   graph_body,[]) in
  let serialize_fun = (return,serialize,[(no_qual (Ident "FILE *"),"arg1");
				       (no_qual (Ident "hash_table *"), "arg2");
				       (no_qual (Ident "unsigned long"), "arg3")],serialize_body,[]) in			
  let deserialize_fun = (return2,deserialize,
			[(no_qual (Ident "FILE *"),"arg1")],
			deserialize_body,[])
  in
  let serialize_rgn_fun = (return, serialize_rgn, [(no_qual (Ident "FILE *"),"arg1");
					      ], serialize_body,[])in
  let deserialize_rgn_fun = (return, deserialize_rgn, [(no_qual (Ident "translation"), "arg1"); (no_qual (Ident "FILE *"),"arg2")],
			     serialize_body,[]) 
  in
  (header#add_gdecls [cmnt;prototype init_fun;prototype reset_fun; 
                      prototype stats_fun;prototype graph_fun; 
		      prototype serialize_fun; prototype deserialize_fun;
		      prototype serialize_rgn_fun; prototype deserialize_rgn_fun;
		      (macro "EXTERN_C_END");endif];
   source#add_fdefs [func init_fun;func reset_fun; func stats_fun;
		    func graph_fun;func serialize_fun;func deserialize_fun;
		    func serialize_rgn_fun; func deserialize_rgn_fun])
    
let to_c (strid,sigid,dataspecs) = 
  let env,header,source = Env.empty_env,empty_header,empty_file in
  let _ = gen_preamble env sigid header source; in
  let env = gen_dataspecs env dataspecs header source; in
  let sorts = Env.exprids_with_sorts env in 
  let _ = gen_postamble env strid header source sorts; in 
  (header,source)
    
let spec_to_c (spec : engspec) (h_file : string) (c_file : string ) = 
  let header,source = to_c spec in
  let header_channel = open_out h_file in
  let c_channel = open_out c_file in
  try 
    header#print header_channel; 
    print_string ("Wrote header file: " ^ h_file ^ "\n");
    source#print c_channel;
    print_string ("Wrote source file: " ^ c_file ^ "\n");
    close_out header_channel;
    close_out c_channel
  with | x -> raise x
  
     

