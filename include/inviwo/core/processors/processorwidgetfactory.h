/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
#include <inviwo/core/util/factory.h>

namespace inviwo {

class IVW_CORE_API ProcessorWidgetFactory
    : public StandardFactory<ProcessorWidget, ProcessorWidgetFactoryObject, std::string_view,
                             Processor*>,
      public Factory<ProcessorWidget, Processor*> {
public:
    using StandardFactory<ProcessorWidget, ProcessorWidgetFactoryObject, std::string_view,
                          Processor*>::create;
    using StandardFactory<ProcessorWidget, ProcessorWidgetFactoryObject, std::string_view,
                          Processor*>::hasKey;

    virtual std::unique_ptr<ProcessorWidget> create(Processor* p) const override {
        return StandardFactory<ProcessorWidget, ProcessorWidgetFactoryObject, std::string_view,
                               Processor*>::create(p->getClassIdentifier(), p);
    };
    virtual bool hasKey(Processor* p) const override {
        return StandardFactory<ProcessorWidget, ProcessorWidgetFactoryObject, std::string_view,
                               Processor*>::hasKey(p->getClassIdentifier());
    }
};

}  // namespace inviwo
