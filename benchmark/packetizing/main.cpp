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
#include <mts/packetizer.hpp>

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
        gauge::config_set cs;
        auto filename = options["filename"].as<std::string>();
        cs.set_value<std::string>("filename", filename);
        boost::iostreams::mapped_file_source file;
        file.open(filename);
        assert(file.is_open());
        cs.set_value<uint32_t>("size", file.size());
        file.close();

        auto packet_size = options["packet_size"].as<uint16_t>();
        cs.set_value<uint16_t>("packet_size", packet_size);

        add_configuration(cs);
    }

    void test_body() override
    {
        gauge::config_set cs = get_current_configuration();
        auto filename = cs.get_value<std::string>("filename");
        boost::iostreams::mapped_file_source file;
        file.open(filename);
        assert(file.is_open());
        std::vector<uint8_t> buffer(file.data(), file.data() + file.size());
        file.close();

        auto packet_size = cs.get_value<uint16_t>("packet_size");

        mts::packetizer packetizer([](auto data, auto size)
        {
            assert(data != nullptr);
            assert(size != 0U);
            assert(data[0] == 0x47);
        });
        const auto packets = buffer.size() / packet_size;
        uint64_t offset = 0;
        RUN
        {
            for (uint32_t i = 0; i < packets; ++i)
            {
                packetizer.read((uint8_t*)buffer.data() + offset, packet_size);
                offset += packet_size;
            }
        }
    }
};

BENCHMARK_F(parsing_benchmark, parsing, h264, 10);

/// Using this macro we may specify options. For specifying options
/// we use the boost program options library. So you may additional
/// details on how to do it in the manual for that library.
BENCHMARK_OPTION(arithmetic_options)
{
    gauge::po::options_description options;

    options.add_options()
    ("filename", gauge::po::value<std::string>()->required(),
     "Set the file name")
    ("packet_size", gauge::po::value<uint16_t>()->default_value(1490U),
     "Packet size");

    gauge::runner::instance().register_options(options);
}

int main(int argc, const char* argv[])
{
    srand(static_cast<uint32_t>(time(0)));

    gauge::runner::add_default_printers();
    gauge::runner::run_benchmarks(argc, argv);

    return 0;
}
