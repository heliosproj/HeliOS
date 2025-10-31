#!/bin/bash

# @file config.sh
# @author Manny Peterson <manny@heliosproj.org>
# @brief A shell script to build Uncrustify configuration file
# 
# @copyright
# HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
#  
#  SPDX-License-Identifier: GPL-2.0-or-later

CONFIG=$(cat <<- END
disable_processing_cmt:"*UNCRUSTIFY-OFF*"
enable_processing_cmt:"*UNCRUSTIFY-ON*"
newlines:lf
utf8_bom:remove
utf8_force:true
sp_arith:force
sp_assign:force
sp_bool:force
sp_compare:force
sp_inside_paren:remove
sp_paren_paren:remove
sp_cparen_oparen:remove
sp_paren_brace:force
sp_brace_brace:remove
sp_before_ptr_star:force
sp_between_ptr_star:remove
sp_after_ptr_star:remove
sp_ptr_star_paren:remove
sp_before_ptr_star_func:force
sp_before_byref:force
sp_after_byref:remove
sp_before_byref_func:force
sp_byref_paren:force
sp_before_sparen:remove
sp_inside_sparen:remove
sp_inside_for:remove
sp_sparen_paren:remove
sp_after_sparen:force
sp_sparen_brace:force
sp_do_brace_open:force
sp_brace_close_while:force
sp_while_paren_open:force
sp_before_semi_for:remove
sp_before_semi_for_empty:remove
sp_between_semi_for_empty:remove
sp_after_semi:force
sp_after_semi_for_empty:remove
sp_before_square:remove
sp_before_squares:remove
sp_inside_square:remove
sp_inside_square_empty:remove
sp_after_comma:force
sp_paren_qualifier:force
sp_after_cast:force
sp_inside_paren_cast:remove
sp_sizeof_paren:remove
sp_inside_braces_enum:remove
sp_inside_braces_struct:remove
sp_inside_braces:remove
sp_inside_braces_empty:remove
sp_type_func:remove
sp_func_proto_paren:remove
sp_func_proto_paren_empty:remove
sp_func_def_paren_empty:remove
sp_inside_fparens:remove
sp_inside_fparen:remove
sp_inside_tparen:remove
sp_after_tparen_close:remove
sp_square_fparen:force
sp_fparen_brace:force
sp_func_call_paren:remove
sp_func_call_paren_empty:remove
sp_attribute_paren:force
sp_defined_paren:remove
sp_macro:force
sp_macro_func:force
sp_else_brace:force
sp_brace_else:force
sp_brace_typedef:force
sp_cond_colon:force
sp_cond_question:force
sp_extern_paren:force
sp_before_tr_cmt:force
sp_num_before_tr_cmt:1
sp_enum_brace:force
sp_enum_assign:force
indent_columns:2
indent_continue:-2
indent_with_tabs:0
indent_extern:true
nl_start_of_file:remove
nl_end_of_file:remove
nl_enum_brace:remove
nl_struct_brace:remove
nl_union_brace:remove
nl_if_brace:remove
nl_brace_else:remove
nl_elseif_brace:remove
nl_else_brace:remove
nl_else_if:remove
nl_before_if_closing_paren:remove
nl_for_brace:remove
nl_while_brace:remove
nl_do_brace:remove
nl_brace_while:remove
nl_switch_brace:remove
nl_before_case:true
nl_enum_own_lines:true
nl_after_semicolon:true
nl_after_brace_open:true
nl_max:2
nl_max_blank_in_func:1
nl_before_func_body_proto:1
nl_before_func_body_def:1
nl_after_func_proto:1
nl_after_func_body:2
nl_var_def_blk_end_func_top:1
nl_var_def_blk_start:1
nl_var_def_blk_end:1
nl_before_block_comment:1
nl_before_c_comment:1
nl_comment_func_def:1
nl_remove_extra_newlines:1
nl_func_var_def_blk:1
eat_blanks_after_open_brace:false
eat_blanks_before_close_brace:false
code_width:160
ls_code_width:true
cmt_width:80
cmt_reflow_mode:2
cmt_convert_tab_to_spaces:true
cmt_align_doxygen_javadoc_tags:true
cmt_star_cont:true
cmt_sp_after_star_cont:1
mod_full_brace_do:force
mod_full_brace_for:force
mod_full_brace_function:force
mod_full_brace_if:force
mod_full_brace_while:force
mod_full_brace_using:force
mod_paren_on_return:force
mod_full_paren_if_bool:true
mod_remove_extra_semicolon:true
mod_add_long_ifdef_endif_comment:1
mod_add_long_ifdef_else_comment:1
pp_indent:force
pp_indent_count:2
pp_if_indent_code:true
END
)

DEFFILE=defaults.cfg
OUTFILE=helios.cfg

# Copy defaults as base
cp $DEFFILE $OUTFILE

# Process each configuration line
IFS=$'\n' command eval "LINES=(\$CONFIG)"
for LINE in "${LINES[@]}"
do
  KEY=$(echo $LINE | cut -f1 -d:)
  VALUE=$(echo $LINE | cut -f2- -d:)
  sed -i "s|^${KEY}[[:space:]]*=.*|${KEY} = ${VALUE}|" $OUTFILE
done

echo "Generated $OUTFILE from $DEFFILE with HeliOS-specific modifications"
