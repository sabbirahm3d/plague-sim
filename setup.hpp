#ifndef PLAGUE_SETUP_HPP
#define PLAGUE_SETUP_HPP

#include "plague.hpp"
#include <math.h>
#include <limits>

plague::plague(SST::ComponentId_t id, SST::Params &params) :
        SST::Component(id),
        // Collect all the parameters from the project driver
        m_clock(params.find<std::string>("CLOCK", "1Hz")),
        seed_lim(params.find<std::string>("SEED", "00000")),
        m_rng(new SST::RNG::MersenneRNG(std::stoi(seed_lim))),
        m_gen(std::stoi(seed_lim)),
        // initialize ram links
        flash_mem_din_link(configureLink("flash_mem_din")),
        flash_mem_dout_link(configureLink(
                "flash_mem_dout",
                new SST::Event::Handler<plague>(this, &plague::flash_mem))
        ),
        // initialize mutation links
        mutation_din_link(configureLink("mutation_din")),
        mutation_dout_link(configureLink(
                "mutation_dout",
                new SST::Event::Handler<plague>(this, &plague::mutation))
        ),
        // initialize limit links
        rng_limit_din_link(configureLink("rng_limit_din")),
        rng_limit_dout_link(configureLink(
                "rng_limit_dout",
                new SST::Event::Handler<plague>(this, &plague::rng_limit))
        ),
        // initialize population infected RNG links
        rng_pop_inf_din_link(configureLink("rng_pop_inf_din")),
        rng_pop_inf_dout_link(configureLink(
                "rng_pop_inf_dout",
                new SST::Event::Handler<plague>(this, &plague::rng_pop_inf))
        ),
        // initialize mutation RNG links
        rng_mut_din_link(configureLink("rng_mut_din")),
        rng_mut_dout_link(configureLink(
                "rng_mut_dout",
                new SST::Event::Handler<plague>(this, &plague::rng_mut))
        ),

        // initialize mutation RNG links
        mul_inv_inf_din_link(configureLink("mul_inv_inf_din")),
        mul_inv_inf_dout_link(configureLink(
                "mul_inv_inf_dout",
                new SST::Event::Handler<plague>(this, &plague::mul_inv_inf))
        ),
        // initialize mutation RNG links
        mul_inv_sev_din_link(configureLink("mul_inv_sev_din")),
        mul_inv_sev_dout_link(configureLink(
                "mul_inv_sev_dout",
                new SST::Event::Handler<plague>(this, &plague::mul_inv_sev))
        ),

        // // initialize infectivity random float links
        // randf_inf_din_link(configureLink("randf_inf_din")),
        // randf_inf_dout_link(configureLink(
        //         "randf_inf_dout",
        //         new SST::Event::Handler<plague>(this, &plague::randf_inf))
        // ),
        // initialize lethality random float links
        randf_fat_din_link(configureLink("randf_fat_din")),
        randf_fat_dout_link(configureLink(
                "randf_fat_dout",
                new SST::Event::Handler<plague>(this, &plague::randf_fat))
        ),
        // // initialize severity random float links
        // randf_sev_din_link(configureLink("randf_sev_din")),
        // randf_sev_dout_link(configureLink(
        //         "randf_sev_dout",
        //         new SST::Event::Handler<plague>(this, &plague::randf_sev))
        // ),
        // initialize birth rate random float links
        randf_br_din_link(configureLink("randf_br_din")),
        randf_br_dout_link(configureLink(
                "randf_br_dout",
                new SST::Event::Handler<plague>(this, &plague::randf_br))
        ),
        // initialize research random float links
        randf_rsrch_din_link(configureLink("randf_rsrch_din")),
        randf_rsrch_dout_link(configureLink(
                "randf_rsrch_dout",
                new SST::Event::Handler<plague>(this, &plague::randf_rsrch))
        ),
        // initialize mul population infected links
        mul_pop_inf_din_link(configureLink("mul_pop_inf_din")),
        mul_pop_inf_dout_link(configureLink(
                "mul_pop_inf_dout",
                new SST::Event::Handler<plague>(this, &plague::mul_pop_inf))
        ),
        // initialize mul batch infected links
        mul_pop_dead_din_link(configureLink("mul_pop_dead_din")),
        mul_pop_dead_dout_link(configureLink(
                "mul_pop_dead_dout",
                new SST::Event::Handler<plague>(this, &plague::mul_pop_dead))
        ),
        // initialize cure threshold floor links
        floor_cure_thresh_din_link(configureLink("floor_cure_thresh_din")),
        floor_cure_thresh_dout_link(configureLink(
                "floor_cure_thresh_dout",
                new SST::Event::Handler<plague>(this, &plague::floor_cure_thresh))
        ),
        // initialize population infected floor links
        floor_pop_inf_din_link(configureLink("floor_pop_inf_din")),
        floor_pop_inf_dout_link(configureLink(
                "floor_pop_inf_dout",
                new SST::Event::Handler<plague>(this, &plague::floor_pop_inf))
        ),
        // initialize population dead floor links
        floor_pop_dead_din_link(configureLink("floor_pop_dead_din")),
        floor_pop_dead_dout_link(configureLink(
                "floor_pop_dead_dout",
                new SST::Event::Handler<plague>(this, &plague::floor_pop_dead))
        ),
        // initialize minimum lethality links
        min_fat_din_link(configureLink("min_fat_din")),
        min_fat_dout_link(configureLink(
                "min_fat_dout",
                new SST::Event::Handler<plague>(this, &plague::min_fat))
        ),
        // initialize minimum infectivity links
        min_inf_din_link(configureLink("min_inf_din")),
        min_inf_dout_link(configureLink(
                "min_inf_dout",
                new SST::Event::Handler<plague>(this, &plague::min_inf))
        ),
        m_cure_threshold(0), m_batch_infected(0), m_total_infected(0), m_total_infected_today(0),
        m_total_dead_today(0), m_gene(0),
        m_severity(0.0), m_infectivity(0.0), m_fatality(0.0), m_birth_rate(0.0), m_cure(0.0), m_research(0.0) {

    m_output.init("\033[93mplague-" + getName() + "\033[0m -> ", 1, 0, SST::Output::STDOUT);

    if (std::stoi(seed_lim) > 65535) {
        m_output.fatal(CALL_INFO, -1, "16-bit seed integer overflow\n");
    } else if (seed_lim.length() != 5) {
        m_output.fatal(CALL_INFO, -1, "Seed integer string must have a length of 5\n");
    }

    // Just register a plain clock for this simple example
    registerClock(m_clock, new SST::Clock::Handler<plague>(this, &plague::tick));

    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

}

void plague::setup() {

    m_output.verbose(CALL_INFO, 1, 0, "Component is being set up.\n");
    m_dis.param(std::uniform_int_distribution<unsigned int>::param_type(5, 100));
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';
    m_dis.param(std::uniform_int_distribution<unsigned int>::param_type(6, 8));
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';
    m_dis.param(std::uniform_int_distribution<unsigned int>::param_type(1, 6));
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';
    std::cout << m_dis(m_gen) << '\n';

    get_seed(seed_sev);
    get_seed(seed_birth_rate);
    get_seed(seed_let);
    get_seed(seed_inf);
    get_seed(seed_pop_inf);
    get_seed(seed_research);
    get_seed(seed_mutation);

}

void plague::finish() {

    m_output.verbose(CALL_INFO, 1, 0, "Destroying %s...\n", getName().c_str());
    std::fclose(m_fp);

}

void plague::get_seed(std::string &seed) {

    seed = std::to_string((uint16_t) m_rng->generateNextUInt32());
    align_signal_width('0', 5, seed);

}

void plague::align_signal_width(const char chr, int width, std::string &signal) {
    int _len = signal.length();
    if (_len < width) {
        signal = std::string(width - _len, chr).append(signal);
    }
}

void plague::append_signal(const char chr, int width, std::string &signal) {
    int _len = signal.length();
    if (_len < width) {
        signal += std::string(width - _len, chr);
    }
}

std::string plague::align_signal_width(int width, float signal) {
    std::ostringstream _data_out;
    _data_out << std::fixed << std::setprecision(width) << signal;
    return _data_out.str().substr(0, width);
}

bool float_less_than(float a, float b) {
    return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon());
}

#endif
