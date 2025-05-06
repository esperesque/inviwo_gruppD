/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/animation/workspaceanimations.h>

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializer.h>             // for Serializer
#include <inviwo/core/network/processornetwork.h>                // for ProcessorNetwork
#include <inviwo/core/network/workspacemanager.h>                // for WorkspaceManager, Worksp...
#include <inviwo/core/util/exception.h>                          // for Exception
#include <inviwo/core/util/typetraits.h>                         // for alwaysTrue, identity
#include <modules/animation/datastructures/animation.h>          // for Animation
#include <modules/animation/datastructures/animationobserver.h>  // for AnimationObservable
#include <modules/animation/datastructures/animationtime.h>      // for Seconds
#include <modules/animation/datastructures/keyframe.h>           // for Keyframe
#include <modules/animation/datastructures/keyframesequence.h>   // for KeyframeSequence
#include <modules/animation/datastructures/track.h>              // for Track
#include <modules/animation/mainanimation.h>                     // for MainAnimation

#include <algorithm>   // for find_if, min
#include <functional>  // for __base
#include <iterator>    // for distance
#include <string>      // for basic_string, string
#include <utility>     // for move

namespace inviwo {

namespace animation {
class AnimationController;
class AnimationManager;

WorkspaceAnimations::WorkspaceAnimations(InviwoApplication* app, AnimationManager& animationManager,
                                         AnimationModule& module)
    : animationManager_{animationManager}
    , animations_{[&]() {
        std::vector<std::unique_ptr<Animation>> res;
        res.push_back(std::make_unique<Animation>(&animationManager_, "Animation 1"));
        return res;
    }()}
    , mainAnimation_(app, *animations_.front(), module)
    , app_{app} {

    animationClearHandle_ = app->getWorkspaceManager()->onClear([&]() { clear(); });
    animationSerializationHandle_ = app->getWorkspaceManager()->onSave([&](Serializer& s) {
        s.serialize("MainAnimationIndex", getMainAnimationIndex());
        s.serialize("Animations", animations_, "Animation");
    });
    animationDeserializationHandle_ = app->getWorkspaceManager()->onLoad([&](Deserializer& d) {
        size_t mainAnimation = 0;
        d.deserialize("MainAnimationIndex", mainAnimation);

        d.deserialize("Animations", animations_, "Animation",
                      deserializer::IndexFunctions{
                          .makeNew =
                              [&]() {
                                  // Must pass AnimationManager to Animation constructor
                                  return std::make_unique<Animation>(&animationManager_);
                              },
                          .onNew = [&](std::unique_ptr<Animation>&,
                                       size_t) { addInternal(--animations_.end()); },
                          .onRemove =
                              [&](std::unique_ptr<Animation>& anim) {
                                  // Previously last element was removed
                                  onChanged_.invoke(size(), *anim);
                              }});

        // Failsafe in case no animation was found
        if (animations_.empty()) {
            add("Animation 1");
        }
        setMainAnimation(*animations_[std::min(mainAnimation, size() - 1)]);
    });

    onChanged_.add([app = app_](size_t, Animation&) { app->getWorkspaceManager()->setModified(); });
}

WorkspaceAnimations::~WorkspaceAnimations() = default;

Animation& animation::WorkspaceAnimations::get(size_t index) { return *animations_.at(index); }

std::vector<Animation*> WorkspaceAnimations::get(std::string_view name) {
    std::vector<Animation*> animations;
    for (auto& elem : animations_) {
        if (elem->getName() == name) {
            animations.push_back(elem.get());
        }
    }
    return animations;
}

Animation& WorkspaceAnimations::operator[](size_t i) { return *animations_[i]; }

const Animation& WorkspaceAnimations::operator[](size_t i) const { return *animations_[i]; }

WorkspaceAnimations::const_iterator WorkspaceAnimations::begin() const {
    return util::makeIndirectIterator(animations_.begin());
}

WorkspaceAnimations::const_iterator WorkspaceAnimations::end() const {
    return util::makeIndirectIterator(animations_.end());
}

WorkspaceAnimations::iterator WorkspaceAnimations::begin() {
    return util::makeIndirectIterator(animations_.begin());
}

WorkspaceAnimations::iterator WorkspaceAnimations::end() {
    return util::makeIndirectIterator(animations_.end());
}

Animation& WorkspaceAnimations::add(std::string_view name) {
    return add(Animation(&animationManager_, name));
}

Animation& WorkspaceAnimations::add(Animation anim) {
    animations_.emplace_back(std::make_unique<Animation>(std::move(anim)));
    addInternal(--animations_.end());
    return *animations_.back();
}

void WorkspaceAnimations::addInternal(
    typename std::vector<std::unique_ptr<Animation>>::iterator aniIt) {
    (*aniIt)->AnimationObservable::addObserver(this);
    for (auto& track : **aniIt) {
        for (size_t seqi = 0; seqi < track.size(); seqi++) {
            for (size_t keyi = 0; keyi < track[seqi].size(); keyi++) {
                track[seqi][keyi].addObserver(this);
            }
            track[seqi].addObserver(this);
        }
        track.addObserver(this);
    }

    onChanged_.invoke(std::distance(animations_.begin(), aniIt), **aniIt);
}

Animation& WorkspaceAnimations::insert(size_t index, std::string_view name) {
    return insert(begin() + index, name);
}

Animation& WorkspaceAnimations::insert(const_iterator position, std::string_view name) {
    auto newIt =
        animations_.insert(position.base(), std::make_unique<Animation>(&animationManager_, name));
    addInternal(newIt);
    return **newIt;
}

void WorkspaceAnimations::erase(size_t index) { erase(begin() + index); }

void WorkspaceAnimations::erase(const_iterator position) {
    // Om bara en animation finns: töm den men behåll platsen
    if (size() == 1) {
        animations_[0]->clear();
        animations_[0]->setName("Animation 1");
        onChanged_.invoke(0, *animations_[0]);
        return;
    }

    auto pos = animations_.begin() + std::distance(animations_.cbegin(), position.base());
    const auto erasedIndex = std::distance(animations_.begin(), pos);  //  <-- spara index

    // Håll MainAnimation vid liv
    if (find(&mainAnimation_.get()).base() == pos) {
        auto newMain = (pos == animations_.begin() ? pos + 1 : pos - 1);
        mainAnimation_.set(**newMain);
    }

    auto anim = std::move(*pos);
    animations_.erase(pos);

    onChanged_.invoke(erasedIndex, *anim);  //  <-- använd sparat index
}


void WorkspaceAnimations::clear() {
    auto nAnimations = static_cast<int>(size()) - 1;
    for (auto i = nAnimations; i >= 0; i--) {
        erase(i);
    }
}

void WorkspaceAnimations::setMainAnimation(Animation& anim) {
    if (auto it = find(&anim); it != end()) {
        mainAnimation_.set(anim);
    } else {
        add(anim);
        mainAnimation_.set(*animations_.back());
    }
}

size_t WorkspaceAnimations::getMainAnimationIndex() const {
    auto it = find(&mainAnimation_.get());
    return std::distance(begin(), it);
}

MainAnimation& WorkspaceAnimations::getMainAnimation() { return mainAnimation_; }

const MainAnimation& WorkspaceAnimations::getMainAnimation() const { return mainAnimation_; }

WorkspaceAnimations::const_iterator WorkspaceAnimations::find(const Animation* anim) const {
    return std::find_if(begin(), end(), [anim](const auto& a) { return anim == &a; });
}

WorkspaceAnimations::iterator WorkspaceAnimations::find(const Animation* anim) {
    return std::find_if(begin(), end(), [anim](const auto& a) { return anim == &a; });
}

void WorkspaceAnimations::onAnyChange() { app_->getWorkspaceManager()->setModified(); }

void WorkspaceAnimations::onAnimationChanged(AnimationController*, Animation*, Animation* newAnim) {
    if (find(newAnim) == end()) {
        // WorkspaceAnimations must own the Animation
        throw Exception("MainAnimation must be set through WorkspaceAnimations");
    }

    onAnyChange();
}

void WorkspaceAnimations::onTrackAdded(Track* track) {
    track->addObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onTrackRemoved(Track* track) {
    track->removeObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onNameChanged(Animation*) { onAnyChange(); }
void WorkspaceAnimations::onKeyframeSequenceAdded(Track*, KeyframeSequence* seq) {
    seq->addObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onKeyframeSequenceRemoved(Track*, KeyframeSequence* seq) {
    seq->removeObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onEnabledChanged(Track*) { onAnyChange(); }
void WorkspaceAnimations::onNameChanged(Track*) { onAnyChange(); }
void WorkspaceAnimations::onPriorityChanged(Track*) { onAnyChange(); }
void WorkspaceAnimations::onKeyframeAdded(Keyframe* key, KeyframeSequence*) {
    key->addObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onKeyframeRemoved(Keyframe* key, KeyframeSequence*) {
    key->removeObserver(this);
    onAnyChange();
}
void WorkspaceAnimations::onKeyframeSequenceMoved(KeyframeSequence*) { onAnyChange(); }
void WorkspaceAnimations::onKeyframeSequenceSelectionChanged(KeyframeSequence*) { onAnyChange(); }
void WorkspaceAnimations::onKeyframeTimeChanged(Keyframe*, Seconds) { onAnyChange(); }
void WorkspaceAnimations::onKeyframeSelectionChanged(Keyframe*) { onAnyChange(); }

}  // namespace animation

}  // namespace inviwo
