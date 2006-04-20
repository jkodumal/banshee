(*
 * Copyright (c) 2006
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
open Fsm

module H = Hashtbl

module Q = Queue

exception DuplicateState of string
exception NoIndex
exception DropN

let annot_name x = "ANNOT_" ^ x
	
let index_of e l = 
	let rec aux n = function
		| h :: t -> if (e = h) then n else (aux (n+1) t)
		| [] -> raise NoIndex
	in
	aux 0 l
	
let rec dropn n  = function
	| h :: t -> if (n = 0)
			   then h::t 
			   else (dropn (n-1) t)
	| [] -> if (not (n = 0)) 
			then raise DropN
			else []      
	
(* Assumes f and g have the same length *)
let compose f g = 
	let result = Array.make (Array.length f) 0 in
	for i = 0 to (Array.length f -1) do
		result.(i) <- f.(g.(i))
	done;
	result

(* Returns the index of the new state *)
let delta (statenums: (string,int) H.t) (s : state) (sym) : int =
	let rec transition = function
		| (Letter sym',s')::t -> if (sym = annot_name sym') 
								 then Some (H.find statenums s')
								 else (transition t)
		| (Param(sym',var),s')::t -> if (sym = annot_name sym') 
									then Some (H.find statenums s')
									else (transition t)
		| (Wildcard,s')::t -> Some (H.find statenums s')
		| [] -> None
	in 
	match (transition s.delta) with
		| Some i -> (i)
		| None -> H.find statenums s.name

let debug_rep_fun f =
		for j = 0 to (Array.length f - 1) do
			Printf.printf "%d " f.(j); 
		done;
		print_string "\n"
		
let debug_rep_funs fl =
	for i = 0 to (Array.length fl - 1) do
		Printf.printf "%d : " i;
		debug_rep_fun fl.(i);
		done;
	print_string "\n"

(* worklist-based algorithm for computing the rep fun set *)
let compute_rep_functions (rep_funs0 : (int array) array) =
	let fset = ref [] in
	let delta_map = H.create 32 in
	let worklist = Q.create() in
	let _ = 
		for i = 0 to (Array.length rep_funs0 - 1)  do
			Q.push rep_funs0.(i) worklist
		done
	in
	while (not (Q.is_empty worklist)) do
		let g = Q.pop worklist in
		let newindex = List.length (!fset) in
		begin
			if (List.mem g !fset) then ()
			else
			 begin
	 			fset := !fset @ [g]; 
				let add_comp_funs f oldindex = 
					let add_new h (i:int) (j:int) = 
						begin
					    	H.add delta_map (i,j) h;
							if (not (List.mem h !fset))
							then Q.push h worklist
							else ()
						end
					in
					let fog = compose f g in
					let gof = compose g f in
			 		begin
						add_new fog oldindex newindex;
						add_new gof newindex oldindex
					end
				in
				let x = ref (-1) in
				List.iter (fun f -> incr x; add_comp_funs f (!x) ) !fset
			end
		end
	done;
	let n = (List.length !fset) in
	let table = Array.make_matrix n n 0 in
    let _ = for i = 0 to n - 1 do
				for j = 0 to n - 1 do
		 			table.(i).(j) <- index_of (H.find delta_map (i,j)) !fset
				done
			done
	in
	(!fset, table)

let init_rep_functions (symtab: (string,int) H.t)
						(statenums : (string,int) H.t)
						 (statemap: state array) 
						automaton header source =					
	let num_states = List.length automaton in	
	let make_rep f_init = Array.init num_states f_init in
	let epsilon_rep = make_rep (fun x -> x) in
	let rep_funs : (int array) array = 
		Array.make (1 + (H.length symtab)) (epsilon_rep) in 
	let singleton_rep (sym) =
		let result = make_rep (fun x -> -1) in
		let _ = for i = 0 to (num_states-1) do
				result.(i) <- delta statenums (statemap.(i)) sym 
				done
		in
		result
	in 
	let _ = H.iter (fun sym -> fun i -> (rep_funs.(i) <- singleton_rep sym)) symtab in
	rep_funs

let assign_symbols (automaton: fsm) hdr src =
	let x = ref (0) in
	let symtab = H.create (32) in
	let ptab = H.create (32) in
	let nptab = H.create (32) in
	let add_p (l,v) = if (H.mem ptab (annot_name l,v))
					  then ()
					  else (H.add ptab (annot_name l,v) !x)
	in
	let add_np l = if (H.mem nptab (annot_name l) )
					  then ()
					  else (H.add nptab (annot_name l) !x)
	in
	let add l = if (H.mem symtab (annot_name l)) 
					then () 
					else (H.add symtab (annot_name l) (incr x; !x))
	in
	let add_sym (s,dest) = match s with
		| Letter l -> add l; add_np l
		| Param (l,v) -> add l; add_p (l,v)
		| Wildcard -> ()
	in
	let _ = List.iter (fun x -> (List.iter add_sym x.delta)) automaton in
    symtab, ptab, nptab
	
let assign_states (automaton: fsm) =
	let num_states = List.length automaton in
	let statemap : state array = 
		Array.make num_states {name="dummy";delta = [];opts = NoOpt}
	in
	let statechk : (string,int) H.t = H.create (32) in 
	let index = ref 0 in
	let add s = if (H.mem statechk s.name)
 				then raise (DuplicateState s.name) 
				else 
				begin
					H.add statechk s.name (!index);
					statemap.(!index) <- s;
					incr index;
				end
	in
	let _ = List.iter (fun x -> add x) automaton in
	statemap,statechk
	
(* build an n x n array representing all function compositions *)
let gen_delta_tab tbl n header source =
	let table = ref "{" in
	for i = 0 to n-1 do
		let row = ref "" in
			begin
				for j = 0 to n-1 do
					row := !row ^ "f_" ^string_of_int(tbl.(i).(j)) ^", "
				done;
				table := !table ^ "\n" ^ !row;
			end
	done;
	table := !table ^ "}";
	source#add_gdecl (var ~init:!table (no_qual (Array (Array(Int,n),n))) "annot_transitions" None)

(* TODO instantiation functions *)
let gen_param_singletons ptab header source =
	let gen_singleton (s,v) i = 
			let istr = string_of_int i in
	        let typ = no_qual (Ident "annotation") in
		    let typ3 = no_qual (Struct "annotation_") in
			let initial = "f_" ^ istr in
			let initial2 = "&f_val_" ^ istr in
		    let initial3 = "{ANNOTATION_PARAM, " ^ istr ^"}" in
			let hdr_decl = "annotation $SYMBOL_param_annotation(char *name);" in
			let names = [("$SYMBOL",s);("$VAR",v)] in 
		begin 
			header#add_gdecl (decl_substitution names hdr_decl);
			source#add_gdecl (var ~init:initial3 typ3 ("f_val_" ^ istr) (None));
			source#add_gdecl (var ~init:initial2 typ ("f_" ^ istr) (None));
		end
	in H.iter gen_singleton ptab

let gen_singletons symtab header source =
	let gen_singleton s i = 
			let istr = string_of_int i in
	        let typ = no_qual (Ident "annotation") in
		    let typ3 = no_qual (Struct "annotation_") in
			let initial = "f_" ^ istr in
			let initial2 = "&f_val_" ^ istr in
		    let initial3 = "{ANNOTATION_SINGLETON, " ^ istr ^"}" in
		begin 
			header#add_tdecl (var typ s (Some Extern));
			source#add_gdecl (var ~init:initial3 typ3 ("f_val_" ^ istr) (None));
			source#add_gdecl (var ~init:initial2 typ ("f_" ^ istr) (None));
			source#add_gdecl (var ~init:initial typ s (None))
		end
	in H.iter gen_singleton symtab
	
let gen_rep_funs fset index header source = 
	let _ = debug_rep_funs (Array.of_list fset) in
	let ind = ref index in
	let gen_rep_fun i = 
		let istr = string_of_int i in
    	let initial1 = "{ANNOTATION_SINGLETON, " ^ istr ^"}" in
		let initial2 = "&f_val_" ^ istr in	    
		let typ1 = no_qual (Struct "annotation_") in
	    let typ2 = no_qual (Ident "annotation") in
		begin 
	  	  source#add_gdecl (var ~init:initial1 typ1 ("f_val_" ^ istr) (None));
		  source#add_gdecl (var ~init:initial2 typ2 ("f_" ^ istr) (None));
		end
	in
	List.iter (fun _ -> gen_rep_fun (!ind); incr ind) fset
	
let gen_preamble header source =
	let h1 = include_header true "annotations.h" in
	let h2 = include_header true "banshee_persist_kinds.h" in
	let h3 = include_header true "gen_annotations.h" in
	begin
		source#add_includes [h3;h2;h1]
	end

let to_c (automaton: fsm) =
	let header, source = empty_header, empty_file
	in
	let symtab,ptab,nptab = assign_symbols automaton header source in
	let statemap, statechk = assign_states automaton in
	let _ = gen_preamble header source in
    let _ = gen_singletons nptab header source in
    let _ = gen_param_singletons ptab header source in		 
    let rep_funs0 : (int array) array = init_rep_functions symtab statechk statemap automaton header source
	in
	let (fset,delta_tbl) = compute_rep_functions rep_funs0 in	
	let n = Array.length rep_funs0 in
	let _ = gen_rep_funs (dropn n fset) n header source in
	let _ = gen_delta_tab delta_tbl (List.length fset) header source in	
	let _ = debug_rep_funs (Array.of_list fset) in 
	(header, source)

let fsm_to_c automaton h_file c_file = 
  let header,source = to_c automaton in
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
