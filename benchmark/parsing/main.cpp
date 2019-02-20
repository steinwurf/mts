// Copyright Steinwurf ApS 2018.
// Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <cassert>
#include <cstdint>
#include <ctime>
#include <memory>
#include <system_error>

#include <gauge/gauge.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <mts/parser.hpp>

/// Benchmark for the initialization of different field implementations
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

        gauge::config_set cs = get_current_configuration();
        auto filename = cs.get_value<std::string>("filename");
        m_file.open(filename);
        assert(m_file.is_open());
    }

    void test_body() override
    {
        auto packet_size = 188;
        mts::parser parser(packet_size);
        uint64_t offset = 0;

        RUN
        {
            for (uint32_t i = 0; i < m_file.size() / packet_size; ++i)
            {
                std::error_code error;
                parser.read((uint8_t*)m_file.data() + offset, error);
                offset += packet_size;
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

    void tear_down() override
    {
        m_file.close();
    }

private:

    boost::iostreams::mapped_file_source m_file;
};

//------------------------------------------------------------------
// SimpleOnline
//------------------------------------------------------------------

BENCHMARK_F(parsing_benchmark, parsing, h264, 5);

/// Using this macro we may specify options. For specifying options
/// we use the boost program options library. So you may additional
/// details on how to do it in the manual for that library.
BENCHMARK_OPTION(arithmetic_options)
{
    gauge::po::options_description options;

    options.add_options()
    ("filename", gauge::po::value<std::string>()->required(), "Set the file name");

    gauge::runner::instance().register_options(options);
}

int main(int argc, const char* argv[])
{
    srand(static_cast<uint32_t>(time(0)));

    gauge::runner::add_default_printers();
    gauge::runner::run_benchmarks(argc, argv);

    return 0;
}
