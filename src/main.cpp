/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated September 24, 2021. Replaces all prior versions.
 *
 * Copyright (c) 2013-2021, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <spine/Debug.h>
#include <spine/spine-sfml.h>

using namespace std;
using namespace spine;
#include <stdio.h>
#include <stdlib.h>

void callback(spAnimationState *state, spEventType type, spTrackEntry *entry, spEvent *event) {
	UNUSED(state);
	const char *animationName = (entry && entry->animation) ? entry->animation->name : 0;

	switch (type) {
		case SP_ANIMATION_START:
			printf("%d start: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_INTERRUPT:
			printf("%d interrupt: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_END:
			printf("%d end: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_COMPLETE:
			printf("%d complete: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_DISPOSE:
			printf("%d dispose: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_EVENT:
			printf("%d event: %s, %s: %d, %f, %s %f %f\n", entry->trackIndex, animationName, event->data->name, event->intValue, event->floatValue,
				   event->stringValue, event->volume, event->balance);
			break;
	}
	fflush(stdout);
}

spSkeletonData *readSkeletonJsonData(const char *filename, spAtlas *atlas, float scale) {
	spSkeletonJson *json = spSkeletonJson_create(atlas);
	json->scale = scale;
	spSkeletonData *skeletonData = spSkeletonJson_readSkeletonDataFile(json, filename);
	if (!skeletonData) {
		printf("%s\n", json->error);
		exit(0);
	}
	spSkeletonJson_dispose(json);
	return skeletonData;
}

spSkeletonData *readSkeletonBinaryData(const char *filename, spAtlas *atlas, float scale) {
	spSkeletonBinary *binary = spSkeletonBinary_create(atlas);
	binary->scale = scale;
	spSkeletonData *skeletonData = spSkeletonBinary_readSkeletonDataFile(binary, filename);
	if (!skeletonData) {
		printf("%s\n", binary->error);
		exit(0);
	}
	spSkeletonBinary_dispose(binary);
	return skeletonData;
}

void testcase(void func(spSkeletonData *skeletonData, spAtlas *atlas),
			  const char *jsonName, const char *binaryName, const char *atlasName,
			  float scale) {
	spAtlas *atlas = spAtlas_createFromFile(atlasName, 0);

	spSkeletonData *skeletonData = readSkeletonBinaryData(binaryName, atlas, scale);
	func(skeletonData, atlas);
	spSkeletonData_dispose(skeletonData);

	skeletonData = readSkeletonJsonData(jsonName, atlas, scale);
	func(skeletonData, atlas);
	spSkeletonData_dispose(skeletonData);

	spAtlas_dispose(atlas);

	UNUSED(jsonName);
}

void treats(spSkeletonData *skeletonData, spAtlas *atlas) {
	UNUSED(atlas);
	spSkeletonBounds *bounds = spSkeletonBounds_create();

	spAnimationStateData *stateData = spAnimationStateData_create(skeletonData);

	SkeletonDrawable *drawable = new SkeletonDrawable(skeletonData, stateData);
	drawable->timeScale = 1;
	drawable->setUsePremultipliedAlpha(true);

	spSkeleton *skeleton = drawable->skeleton;
	spSkeleton_setToSetupPose(skeleton);

	skeleton->x = 320;
	skeleton->y = 390;
	spSkeleton_updateWorldTransform(skeleton);

	spSlot *headSlot = spSkeleton_findSlot(skeleton, "head");

	drawable->state->listener = callback;
	spAnimationState_setAnimationByName(drawable->state, 0, "falling", true);

	spAnimationState_update(drawable->state, 0.016);
	spSkeleton_setBonesToSetupPose(drawable->skeleton);
	spAnimationState_apply(drawable->state, drawable->skeleton);
	spSkeleton_updateWorldTransform(drawable->skeleton);

	sf::RenderWindow window(sf::VideoMode(640, 640), "Spine SFML - spineboy");
	window.setFramerateLimit(60);
	sf::Event event;
	sf::Clock deltaClock;
	while (window.isOpen()) {
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) window.close();

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();

		spSkeletonBounds_update(bounds, skeleton, true);
		sf::Vector2i position = sf::Mouse::getPosition(window);

		spSkeleton_setToSetupPose(skeleton);
		drawable->update(delta);

		window.clear();
		window.draw(*drawable);
		window.display();
	}

	spSkeletonBounds_dispose(bounds);
}

int main() {
	testcase(treats, "../treats/export/skeleton.json", "../treats/export/skeleton.skel", "../treats/export/skeleton.atlas", 1.0f);
	return 0;
}
