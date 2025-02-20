/*
 * Copyright 2021 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <api/cli/DaphneUserConfig.h>

#include <vector>
#include <iostream>
#include <memory>

#include "IContext.h"

// This macro is intended to be used in kernel function signatures, such that
// we can change the ubiquitous DaphneContext parameter in a single place, if
// required.
#define DCTX(varname) DaphneContext * varname

/**
 * @brief This class carries all kinds of run-time context information.
 * 
 * An instance of this class is passed to every kernel at run-time. It allows
 * the kernel to retrieve information about the run-time environment.
 */
struct DaphneContext {
    // Feel free to extend this class with any kind of run-time information
    // that might be relevant to some kernel. Each kernel can extract the
    // information it requires and does not need to worry about information it
    // does not require.
    // If you need to add a bunch of related information items, please consider
    // creating an individual struct/class for them and adding a single member
    // of that type here, in order to separate concerns and allow a  high-level
    // overview of the context information.


    std::vector<std::unique_ptr<IContext>> cuda_contexts;

    /**
     * @brief The user configuration (including information passed via CLI
     * arguments etc.).
     *
     * Modifying the configuration is intensionally allowed, since it enables
     * changing the configuration at run-time via DaphneDSL.
     */
    DaphneUserConfig& config;

    explicit DaphneContext(DaphneUserConfig& config) : config(config) {
        //
    }

    ~DaphneContext() {
        for (auto& ctx : cuda_contexts) {
            ctx->destroy();
        }
        cuda_contexts.clear();
    }

#ifdef USE_CUDA
    // ToDo: in a multi device setting this should use a find call instead of a direct [] access
    [[nodiscard]] IContext* getCUDAContext(size_t dev_id) const {
        return cuda_contexts[dev_id].get();
    }
#endif

    [[nodiscard]] bool useCUDA() const { return !cuda_contexts.empty(); }
    
    [[maybe_unused]] [[nodiscard]] DaphneUserConfig getUserConfig() const { return config; }
};
