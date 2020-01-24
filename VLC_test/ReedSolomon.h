#pragma once

/*
(**************************************************************************)
(*                                                                        *)
(*                                Schifra                                 *)
(*                Reed-Solomon Error Correcting Code Library              *)
(*                                                                        *)
(* Release Version 0.0.1                                                  *)
(* http://www.schifra.com                                                 *)
(* Copyright (c) 2000-2014 Arash Partow, All Rights Reserved.             *)
(*                                                                        *)
(* The Schifra Reed-Solomon error correcting code library and all its     *)
(* components are supplied under the terms of the General Schifra License *)
(* agreement. The contents of the Schifra Reed-Solomon error correcting   *)
(* code library and all its components may not be copied or disclosed     *)
(* except in accordance with the terms of that agreement.                 *)
(*                                                                        *)
(* URL: http://www.schifra.com/license.html                               *)
(*                                                                        *)
(**************************************************************************)
*/


/*
Description: This example will demonstrate how to instantiate a Reed-Solomon
encoder and decoder, add the full amount of possible errors,
correct the errors, and output the various pieces of relevant
information.
*/


#include <cstddef>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "schifra_galois_field.hpp"
#include "schifra_galois_field_polynomial.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_reed_solomon_block.hpp"
#include "schifra_error_processes.hpp"

/* Finite Field Parameters */
const std::size_t field_descriptor = 4;
const std::size_t generator_polynommial_index = 0;
const std::size_t generator_polynommial_root_count = 7;

/* Reed Solomon Code Parameters */
const std::size_t code_length = 15; //(2^4 - 1)
const std::size_t fec_length = 7;
const std::size_t data_length = code_length - fec_length;

using namespace std;

class ReedSolomon
{
	// assume the message already on the rigth block size
	// block size here is 0
	vector<int> encode(vector<int> data)
	{
		/* Instantiate Finite Field and Generator Polynomials */
		schifra::galois::field field(field_descriptor,
			schifra::galois::primitive_polynomial_size06,
			schifra::galois::primitive_polynomial06);

		schifra::galois::field_polynomial generator_polynomial(field);

		schifra::sequential_root_generator_polynomial_creator(field,
			generator_polynommial_index,
			generator_polynommial_root_count,
			generator_polynomial);

		/* Instantiate Encoder and Decoder (Codec) */
		schifra::reed_solomon::encoder<code_length, fec_length> encoder(field, generator_polynomial);
		
		/* Instantiate RS Block For Codec */
		schifra::reed_solomon::block<code_length, fec_length> block;
		memset(block.data, 0, code_length);
		for (int i = 0; i < data.size(); i++)
		{
			block[i] = data[i];
		}

		vector<int> result;
		/* Transform message into Reed-Solomon encoded codeword */
		if (encoder.encode(block))
		{
			for (int i = 0; i < data.size(); i++)
			{
				result.push_back(block.data[i]);
			}
			for (int i = 0; i < fec_length; i++)
			{
				result.push_back(block.data[data_length + i]);
			}
		}
		else
		{
			std::cout << "Error - Critical encoding failure!" << std::endl;
		}

		return result;
	}

	// data are already organized in the block format 
	// block size is 4 bits
	// the return value is also in the same format
	vector<int> decode(vector<int> data)
	{
		if (data.size() <= fec_length)
		{
			return vector<int>();
		}
		/* Instantiate Finite Field and Generator Polynomials */
		schifra::galois::field field(field_descriptor,
			schifra::galois::primitive_polynomial_size01,
			schifra::galois::primitive_polynomial01);

		schifra::galois::field_polynomial generator_polynomial(field);

		schifra::sequential_root_generator_polynomial_creator(field,
			generator_polynommial_index,
			generator_polynommial_root_count,
			generator_polynomial);

		/* Instantiate Encoder and Decoder (Codec) */
		schifra::reed_solomon::decoder<code_length, fec_length> decoder(field, generator_polynommial_index);


		/* Instantiate RS Block For Codec */
		schifra::reed_solomon::block<code_length, fec_length> block;
		for (int i = 0; i < data.size() - fec_length; i++)
		{
			block.data[i] = data[i];
		}
		for (int i = 0; i < fec_length; i++)
		{
			block.data[data_length + i] = data[data.size() - fec_length + i];
		}
		vector<int> message;
		if (decoder.decode(block))
		{
			for (int i = 0; i < data.size() - fec_length; i++)
			{
				message.push_back(block.data[i]);
			}
		}
		else
		{
			std::cout << "Error - Critical decoding failure!" << std::endl;
			for (int i = 0; i < data.size() - fec_length; i++)
			{
				message.push_back(data[i]);
			}
		}
		return message;
	}
public:
	// encodes a bit stream using reed solomon code
	// start by dividing the bit stream into chunks of data_length * field_descriptor
	// then encode each chunk of them and get the result back added in the result data
	vector<short> encode_bit_stream(vector<short> data)
	{
		vector<short> encode_result;
		for (int i = 0; i < data.size();)
		{
			vector<int> temp_data;
			for (int j = 0; j < data_length && i < data.size(); j++)
			{
				int tmp_val = 0;
				for (int k = 0; k < field_descriptor && i < data.size(); k++, i++)
				{
					tmp_val = (tmp_val << 1) | (data[i] & 1);
				}
				temp_data.push_back(tmp_val);
			}
			vector<int> temp_encoded = encode(temp_data);
			// get the bits again
			for (int j = 0; j < temp_encoded.size(); j++)
			{
				for (int k = field_descriptor - 1; k >= 0; k--)
				{
					encode_result.push_back((temp_encoded[j] >> k) & 1);
				}
			}
		}
		return encode_result;
	}
	// decodes a bit stream using reed solomon code
	// start by dividing the bit stream into chunks of code_length * field_descriptor
	// then decode each chunk of them and get the result back added in the result data
	vector<short> decode_bit_stream(vector<short> data)
	{
		vector<short> decode_result;
		for (int i = 0; i < data.size();)
		{
			vector<int> temp_data;
			for (int j = 0; j < code_length && i < data.size(); j++)
			{
				int tmp_val = 0;
				for (int k = 0; k < field_descriptor && i < data.size(); k++, i++)
				{
					tmp_val = (tmp_val << 1) | (data[i] & 1);
				}
				temp_data.push_back(tmp_val);
			}
			vector<int> temp_decoded = decode(temp_data);
			// get the bits again
			for (int j = 0; j < temp_decoded.size(); j++)
			{
				for (int k = field_descriptor - 1; k >= 0; k--)
				{
					decode_result.push_back((temp_decoded[j] >> k) & 1);
				}
			}
		}
		return decode_result;
	}
};
