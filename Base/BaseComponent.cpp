#include "BaseComponent.h"

std::size_t	BaseComponent::ClassHashCode = std::hash<std::string>()(TO_STRING(BaseComponent));