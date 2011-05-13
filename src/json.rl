%%{
	machine json;
	
	begin_array = "[" >A_begin_array;
	end_array = "]" >A_end_array;
	begin_object = "{" >A_begin_object;
	end_object = "}" >A_end_object;
	integer = ("-"? [0-9]+) %A_save_int;
	float = ("-"? [0-9]+ (("." [0-9]+) | ("." [0-9]+) (("e" | "E") ("+" | "-")? [0-9]+) | (("e" | "E") ("+" | "-")? [0-9]+))) %A_save_float; 
	num = (integer | float) >A_save_start;
	sp = " "*;
	razd = ",";
	s_r_s = sp razd sp;
	uni = 0x80..0xbf;
	esc_seq = 	"\\\"" | "\\b" | "\\n" | "\\r" | "\\\\" | 
				"\\/" | "\\f" | "\\t" | ("\\u" xdigit{4});
	uni_ascii_area = ((0x20..0x7e) -- (0x22 | 0x5c)) | esc_seq;
	uni_area_1 = 0xc0..0xdf uni;
	uni_area_2 = 0xe0..0xef uni uni;
	uni_area_3 = 0xf0..0xf7 uni uni uni;
	uni_token = uni_ascii_area | uni_area_1 | uni_area_2 | uni_area_3;
	string = "\"" uni_token*  >A_save_start %A_save_string "\"";
	json_value = begin_array  | begin_object | num | string |
			( "null" %to(A_save_null) ) | ( "true" %to(A_save_true) ) | ( "false" %to(A_save_false) );
	non_plain_json_value = begin_array | begin_object;
	name = '"' uni_token* >A_save_start %A_save_name '"'sp ":" sp ;
	object := (sp ((name json_value s_r_s)* name json_value sp)? end_object) $lerr(A_err);
	array := (sp ((json_value s_r_s)* json_value sp)? end_array) $lerr(A_err);
}%%