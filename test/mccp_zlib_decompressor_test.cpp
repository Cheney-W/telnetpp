#include <telnetpp/options/mccp/zlib/decompressor.hpp>
#include <gtest/gtest.h>
#include <zlib.h>
#include <algorithm>
#include <random>

using namespace telnetpp::literals;

namespace {

class a_zlib_decompressor : public testing::Test
{
protected:
    telnetpp::options::mccp::zlib::decompressor zlib_decompressor_;
};

}

TEST_F(a_zlib_decompressor, is_a_codec)
{
    // MCCP will rely on the codec interface, so zlib_decompressor must
    // implement that.
    telnetpp::options::mccp::codec &decompressor = zlib_decompressor_;
}

namespace {

class an_unstarted_zlib_decompressor : public a_zlib_decompressor
{
protected:
    void decompress_data(telnetpp::bytes data)
    {
        zlib_decompressor_(
            data,
            [&](telnetpp::bytes data, bool decompression_ended)
            {
                decompression_ended_ = decompression_ended;
                received_data_.insert(
                    received_data_.end(), data.begin(), data.end());
            });
    }

    std::vector<telnetpp::byte> received_data_;
    bool decompression_ended_ = false;
};

}

TEST_F(an_unstarted_zlib_decompressor, does_not_decompress_data)
{
    auto const data = "test_data"_tb;

    decompress_data(data);

    auto const expected_data = std::vector<telnetpp::byte>{
        data.begin(), data.end()
    };

    ASSERT_EQ(expected_data, received_data_);
    ASSERT_FALSE(decompression_ended_);
}

namespace {

class a_started_zlib_decompressor : public an_unstarted_zlib_decompressor
{
protected:
    a_started_zlib_decompressor()
    {
        zlib_decompressor_.start();
    }
};

}

TEST_F(a_started_zlib_decompressor, decompresses_received_data)
{
    // This test begins a series of somewhat white-box tests, because in 
    // order to test whether decompression occurs, we must first compress.
    z_stream stream = {};
    auto response = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    assert(response == Z_OK);

    telnetpp::byte output_buffer[1023];
    auto const test_data = "datadatadatadatadatadata"_tb;

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_SYNC_FLUSH);
    assert(response == Z_OK);

    auto const compressed_stream = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream);

    auto const expected_data = std::vector<telnetpp::byte>{
        test_data.begin(), test_data.end()
    };

    ASSERT_EQ(expected_data, received_data_);
    ASSERT_FALSE(decompression_ended_);

    deflateEnd(&stream);
}

TEST_F(a_started_zlib_decompressor, announces_the_end_of_a_compression_stream)
{
    z_stream stream = {};
    auto response = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    assert(response == Z_OK);

    telnetpp::byte output_buffer[1023];
    auto const test_data = "datadatadatadatadatadata"_tb;

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_FINISH);
    assert(response == Z_STREAM_END);

    auto const compressed_stream = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream);

    auto const expected_data = std::vector<telnetpp::byte>{
        test_data.begin(), test_data.end()
    };

    ASSERT_EQ(expected_data, received_data_);
    ASSERT_TRUE(decompression_ended_);

    deflateEnd(&stream);
}

TEST_F(a_started_zlib_decompressor, acts_as_if_a_new_stream_after_restarting_decompression_after_decompression_end)
{
    z_stream stream = {};
    auto response = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    assert(response == Z_OK);

    telnetpp::byte output_buffer[1023];
    auto const test_data = "datadatadatadatadatadata"_tb;

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_FINISH);
    assert(response == Z_STREAM_END);

    auto const compressed_stream0 = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream0);
    received_data_.clear();

    response = deflateReset(&stream);
    assert(response == Z_OK);
    zlib_decompressor_.start();

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_SYNC_FLUSH);
    assert(response == Z_OK);

    auto const compressed_stream1 = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream1);

    auto const expected_data = std::vector<telnetpp::byte>{
        test_data.begin(), test_data.end()
    };

    ASSERT_EQ(expected_data, received_data_);
    ASSERT_FALSE(decompression_ended_);

    deflateEnd(&stream);
}

TEST_F(a_started_zlib_decompressor, acts_as_if_a_new_stream_after_restarting_decompression_after_decompression_finish)
{
    z_stream stream = {};
    auto response = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    assert(response == Z_OK);

    telnetpp::byte output_buffer[1023];
    auto const test_data = "datadatadatadatadatadata"_tb;

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_SYNC_FLUSH);
    assert(response == Z_OK);

    auto const compressed_stream0 = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream0);
    received_data_.clear();

    response = deflateReset(&stream);
    assert(response == Z_OK);

    zlib_decompressor_.finish([](auto &&, bool){});
    zlib_decompressor_.start();

    stream.avail_in  = test_data.size();
    stream.next_in   = const_cast<telnetpp::byte *>(test_data.data());
    stream.avail_out = sizeof(output_buffer);
    stream.next_out  = output_buffer;

    response = deflate(&stream, Z_SYNC_FLUSH);
    assert(response == Z_OK);

    auto const compressed_stream1 = telnetpp::bytes{
        output_buffer, stream.next_out
    };

    decompress_data(compressed_stream1);

    auto const expected_data = std::vector<telnetpp::byte>{
        test_data.begin(), test_data.end()
    };

    ASSERT_EQ(expected_data, received_data_);
    ASSERT_FALSE(decompression_ended_);

    deflateEnd(&stream);
}
