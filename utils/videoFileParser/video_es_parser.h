/*
Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <vector>

// Jefftest 
#define BS_RING_SIZE (16 * 1024 * 1024)
//#define BS_RING_SIZE (8 * 1024 * 1024)
//#define BS_RING_SIZE (900 * 1024)
//#define BS_RING_SIZE (400 * 1024)
#define INIT_PIC_DATA_SIZE (2 * 1024 * 1024)

class RocVideoESParser {
    public:
        RocVideoESParser(const char *input_file_path);
        RocVideoESParser();
        ~RocVideoESParser();


        /*! \brief Function to retrieve the bitstream of a picture
         * \param [out] p_pic_data Pointer to the picture data
         * \param [out] pic_size Size of the picture in bytes
         */
        int GetPicData(uint8_t **p_pic_data, int *pic_size);

    private:
        FILE *p_stream_file_ = NULL;

        // Bitstream ring buffer
        uint8_t *bs_ring_;
        uint32_t read_ptr_; /// start position of unprocessed stream in the ring
        uint32_t write_ptr_;  /// end position of unprocessed stream in the ring
        bool end_of_file_;
        bool end_of_stream_;
        int curr_byte_offset_;
        // AVC/HEVC
        int num_start_code_;
        int curr_start_code_offset_;
        int next_start_code_offset_;
        //int nal_unit_size_;
        // AV1
        int obu_byte_offset_; // header offset
        int obu_size_; // including header
        int num_td_obus_; // number of temporal delimiter OBUs

        // Picture data (linear buffer)
        std::vector<uint8_t> pic_data_;
        int pic_data_size_;
        // AVC/HEVC
        int curr_pic_end_;
        int next_pic_start_;
        int num_pictures_;
        // AV1
        int num_temp_units_; // number of temporal units

        bool ivf_file_header_read_; // indicator if IVF file header has been checked

        /*! \brief Function to retrieve the bitstream of a picture for AVC/HEVC
         * \param [out] p_pic_data Pointer to the picture data
         * \param [out] pic_size Size of the picture in bytes
         */
        int GetPicDataAvcHevc(uint8_t **p_pic_data, int *pic_size);

        /*! \brief Function to retrieve the bitstream of a temporal unit for AV1
         * \param [out] p_pic_data Pointer to the picture data
         * \param [out] pic_size Size of the picture in bytes
         */
        int GetPicDataAv1(uint8_t **p_pic_data, int *pic_size);

        /*! \brief Function to retrieve the bitstream of a temporal unit for AV1 from IVF container
         * \param [out] p_pic_data Pointer to the picture data
         * \param [out] pic_size Size of the picture in bytes
         */
        int GetPicDataIvfAv1(uint8_t **p_pic_data, int *pic_size);

        /*! \brief Function to read bitstream from file and fill into the ring buffer.
        * \return Number of bytes read from file.
        */
        int FetchBitStream();

        /*! \brief Function to check the remaining data size in the ring buffer
         * \return Number of bytes still available in the ring
         */
        int GetDataSizeInRB();

        /*! \brief Function to read one byte from the ring buffer without advancing the read pointer
         * \param [in] offset The byte offset to read
         * \param [out] data The byte read
         * \return True: success; False: no more byte available.
         */
        bool GetByte(int offset, uint8_t *data);

        /*! \brief Function to read the specified bytes from the ring buffer without advancing the read pointer
         * \param [in] offset The starting byte offset to read
         * \param [in] size The numbers of bytes to read
         * \param [out] data The bytes read
         * \return True: success; False: can not read the set bytes
         */
        bool ReadBytes(int offset, int size, uint8_t *data);

        /*! \brief Function to update the read pointer by the set bytes
         * \param [in] value The new read pointer value
         */
        void SetReadPointer(int value);

        /*! \brief Function to find the start codes from the ring buffer to locate the NAL units
        * \return Returns: true: a new start code is found or end of stream reached; false: no start code found. 
        */
        bool FindStartCode();

        /*! \brief Function to check if an HEVC NAL is the (first) slice of a picture
         * \param [in] start_code_offset Start code location of the NAL unit
         * \param [out] slice_flag Slice NAL unit indicator
         * \param [out] first_slice_flag First slice indicator
         */
        void CheckHevcNalForSlice(int start_code_offset, int *slice_flag, int *first_slice_flag);

        /*! \brief Function to check if an AVC NAL is the (first) slice of a picture
         * \param [in] start_code_offset Start code location of the NAL unit
         * \param [out] slice_flag Slice NAL unit indicator
         * \param [out] first_slice_flag First slice indicator
         */
        void CheckAvcNalForSlice(int start_code_offset, int *slice_flag, int *first_slice_flag);

        /*! \brief Function to copy a NAL unit from the bitstream ring buffer to the linear picture data buffer
         */
        void CopyNalUnitFromRing();

        /*! \brief Function to parse an OBU header and size
        * \param [out] obu_type Pointer to the returned OBU type
        * \return true if success
        */
        bool ReadObuHeaderAndSize(int *obu_type);
    
        /*! \brief Function to copy an OBU from the bitstream ring buffer to the linear picture data buffer
         * \return true if success
         */
        bool CopyObuFromRing();

        /*! \brief Function to check the 32 byte stream for IVF file header identity
         * \return true if IVF file header is identified; false: otherwise
         */
        bool CheckIvfFileHeader(uint8_t *stream);

};