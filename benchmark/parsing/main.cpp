// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <cassert>
#include <cstdint>
#include <ctime>
#include <memory>
#include <system_error>

#include <gauge/gauge.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <mts/parser.hpp>

class parsing_benchmark : public gauge::time_benchmark
{
public:

    double measurement() override
    {
        // Get the time spent per iteration
        double time = gauge::time_benchmark::measurement();

        gauge::config_set cs = get_current_configuration();
        auto size = cs.get_value<uint32_t>("size");

        return size / time; // MB/s for each iteration
    }

    std::string unit_text() const override
    {
        return "MB/s";
    }

    void store_run(tables::table& results) override
    {
        if (!results.has_column("throughput"))
            results.add_column("throughput");

        results.set_value("throughput", measurement());
    }

    void get_options(gauge::po::variables_map& options) override
    {
        auto filename = options["filename"].as<std::string>();
        gauge::config_set cs;
        cs.set_value<std::string>("filename", filename);
        boost::iostreams::mapped_file_source file;
        file.open(filename);
        assert(file.is_open());
        cs.set_value<uint32_t>("size", file.size());
        file.close();

        add_configuration(cs);
    }

    void setup() override
    {
        if (m_buffer.empty())
        {
            gauge::config_set cs = get_current_configuration();
            auto filename = cs.get_value<std::string>("filename");
            boost::iostreams::mapped_file_source file;
            file.open(filename);
            assert(file.is_open());
            m_buffer.insert(m_buffer.begin(), file.data(), file.data() + file.size());
            file.close();
        }
    }

    void test_body() override
    {
        RUN
        {
            mts::parser parser;
            uint64_t offset = 0;
            const auto packets = m_buffer.size() / mts::parser::packet_size();
            for (uint32_t i = 0; i < packets; ++i)
            {
                std::error_code error;
                parser.read((uint8_t*)m_buffer.data() + offset, error);
                offset += mts::parser::packet_size();
                if (parser.has_pes())
                {
                    auto pid = parser.pes_pid();
                    mts::stream_type type = (mts::stream_type)parser.stream_type(pid);
                    if (type != mts::stream_type::avc_video_stream)
                        continue;

                    auto& pes_data = parser.pes_data();
                    std::error_code error;
                    auto pes = mts::pes::parse(pes_data.data(), pes_data.size(), error);
                    if (error)
                        continue;

                    assert(pes->payload_data() != nullptr);
                    assert(pes->payload_size() != 0U);
                }
            }
        }
    }

private:

    std::vector<uint8_t> m_buffer;
};

BENCHMARK_F(parsing_benchmark, parsing, h264, 5);

/// Using this macro we may specify options. For specifying options
/// we use the boost program options library. So you may additional
/// details on how to do it in the manual for that library.
BENCHMARK_OPTION(arithmetic_options)
{
    gauge::po::options_description options;

    options.add_options()
    ("filename", gauge::po::value<std::string>()->default_value("test.ts"),
     "Set the file name");

    gauge::runner::instance().register_options(options);
}

int main(int argc, const char* argv[])
{
    srand(static_cast<uint32_t>(time(0)));

    gauge::runner::add_default_printers();
    gauge::runner::run_benchmarks(argc, argv);

    return 0;
}
